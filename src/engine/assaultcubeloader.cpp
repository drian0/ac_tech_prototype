// assaultcubeloader.cpp, based on cubeloader.cpp from Sauerbraten
// supports importing maps in format 10 from AssaultCube v1.3 

#include "engine.h"
#include "game.h"

VAR(importcuberemip, 0, 1024, 2048);

struct cubeloader
{
    enum
    {
        DEFAULT_LIQUID = 1,
        DEFAULT_WALL,
        DEFAULT_FLOOR,
        DEFAULT_CEIL
    };

    enum                              // block types, order matters!
    {
        C_SOLID = 0,                  // entirely solid cube [only specifies wtex]
        C_CORNER,                     // half full corner of a wall
        C_FHF,                        // floor heightfield using neighbour vdelta values
        C_CHF,                        // idem ceiling
        C_SPACE,                      // entirely empty cube
        C_SEMISOLID,                  // generated by mipmapping
        C_MAXTYPE
    };

    enum { HX_UNUSED = 0, HX_MAPINFO, HX_MODEINFO, HX_ARTIST, HX_EDITUNDO, HX_CONFIG, HX_VANTAGEPOINT, HX_NUM, HX_TYPEMASK = 0x3f, HX_FLAG_PERSIST = 0x40 };
    
    #define SMALLEST_FACTOR 6               // determines number of mips there can be        
    #define DEFAULT_FACTOR 8
    #define LARGEST_FACTOR 11               // 10 is already insane
    #define MAXENTITIES 65535
    #define MAXHEADEREXTRA (1<<20)
    #define SWS(w,x,y,s) (&(w)[((y)<<(s))+(x)])
    #define SW(w,x,y) SWS(w,x,y,sfactor)
    #define S(x,y) SW(world,x,y) 

    struct c_sqr
    {
        uchar type;                 // one of the above
        char floor, ceil;           // height, in cubes
        uchar wtex, ftex, ctex;     // wall/floor/ceil texture ids
        uchar r, g, b;              // light value at upper left vertex
        uchar vdelta;               // vertex delta, used for heightfield cubes
        char defer;                 // used in mipmapping, when true this cube is not a perfect mip
        char occluded;              // true when occluded
        uchar utex;                 // upper wall tex id
        uchar tag;                  // used by triggers
        uchar visible;              // temporarily used to flag the visibility of a cube (INVISWTEX, INVISUTEX, INVISIBLE)
        uchar reserved;
    };

    struct c_persistent_entity        // map entity
    {
        short x, y, z;              // cube aligned position
        short attr1;
        uchar type;                 // type is one of the above
        uchar attr2, attr3, attr4;
        short attr5;
        char attr6;
        unsigned char attr7;
    };

    enum                            // static entity types
    {
        C_NOTUSED = 0,                // entity slot not in use in map (usually seen at deleted entities)
        C_LIGHT,                      // lightsource, attr1 = radius, attr2 = intensity (or attr2..4 = r-g-b)
        C_PLAYERSTART,                // attr1 = angle, attr2 = team
        C_I_CLIPS, C_I_AMMO, C_I_GRENADE, // attr1 = elevation
        C_I_HEALTH, C_I_HELMET, C_I_ARMOUR, C_I_AKIMBO,
        C_MAPMODEL,                   // attr1 = angle, attr2 = idx, attr3 = elevation, attr4 = texture, attr5 = pitch, attr6 = roll
        C_CARROT,                     // attr1 = tag, attr2 = type
        C_LADDER,                     // attr1 = height
        C_CTF_FLAG,                   // attr1 = angle, attr2 = red/blue
        C_SOUND,                      // attr1 = idx, attr2 = radius, attr3 = size, attr4 = volume
        C_CLIP,                       // attr1 = elevation, attr2 = xradius, attr3 = yradius, attr4 = height, attr6 = slope, attr7 = shape
        C_PLCLIP,                     // attr1 = elevation, attr2 = xradius, attr3 = yradius, attr4 = height, attr6 = slope, attr7 = shape
        C_DUMMYENT,                   // temporary entity without any function - will not be saved to map files, used to mark positions and for scripting
        C_MAXENTTYPES
    };

    enum
    {
        CORNERTYPE_NONE = -1,
        CORNERTYPE_BETWEEN_FLOOR_CEIL,
        CORNERTYPE_OUTSIDE_FLOOR_CEIL
    };

    struct c_header                   // map file format header
    {
        char head[4];               // "CUBE"
        int version;                // any >8bit quantity is little endian
        int headersize;             // sizeof(header)
        int sfactor;                // in bits
        int numents;
        char maptitle[128];
        uchar texlists[3][256];
        int waterlevel;
        uchar watercolor[4];
        int maprevision;
        int ambient;
        int flags;                  // MHF_*
        int timestamp;              // UTC unixtime of time of save (yes, this will break in 2038)
        int reserved[10];
    };

    struct acmapmodel
    {
        int r, h, z;
        char* name;
    };

    struct actexture
    {
        int scale;
        char* name;
    };

    c_sqr *world;
    int ssize;
    int sfactor;
    int x0, x1, y0, y1, z0, z1;
    c_sqr *o[4];
    int lastremip;
    int progress;
    vector<acmapmodel> acmapmodels;
    vector<actexture> actextures;

    // act as if you did not see this
    char* replace(
        char const* const original,
        char const* const pattern,
        char const* const replacement) {
        size_t const replen = strlen(replacement);
        size_t const patlen = strlen(pattern);
        size_t const orilen = strlen(original);

        size_t patcnt = 0;
        const char* oriptr;
        const char* patloc;

        // find how many times the pattern occurs in the original string
        for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
        {
            patcnt++;
        }

        {
            // allocate memory for the new string
            size_t const retlen = orilen + patcnt * (replen - patlen);
            char* const returned = (char*)malloc(sizeof(char) * (retlen + 1));

            if (returned != NULL)
            {
                // copy the original string, 
                // replacing all the instances of the pattern
                char* retptr = returned;
                for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
                {
                    size_t const skplen = patloc - oriptr;
                    // copy the section until the occurence of the pattern
                    strncpy(retptr, oriptr, skplen);
                    retptr += skplen;
                    // copy the replacement 
                    strncpy(retptr, replacement, replen);
                    retptr += replen;
                }
                // copy the rest of the string.
                strcpy(retptr, oriptr);
            }
            return returned;
        }
    }

    void ac_texture(int* scale, char* name)
    {
        actexture mdl;
        mdl.scale = *scale;
        mdl.name = newstring(name);
        actextures.add(mdl);
    }

    void ac_mapmodel(int* r, int* h, int* z, int* dummy, char* name)
    {
        acmapmodel mdl;
        mdl.r = *r;
        mdl.h = *h;
        mdl.z = *z;
        mdl.name = newstring(name);
        acmapmodels.add(mdl);
    }

    void create_ent(c_persistent_entity &ce)
    {        
        int newtype;
        switch(ce.type)
        {
            case C_LIGHT: newtype = LIGHT; break;
            case C_PLAYERSTART: newtype = PLAYERSTART; break;
            case C_CTF_FLAG: newtype = FLAG; break;
            case C_I_CLIPS: newtype = CLIPS; break;
            case C_I_AMMO: newtype = AMMO; break;
            case C_I_GRENADE: newtype = GRENADE; break;
            case C_I_HEALTH: newtype = HEALTH; break;
            case C_I_HELMET: newtype = HELMET; break;
            case C_I_ARMOUR: newtype = ARMOUR; break;
            case C_I_AKIMBO: newtype = AKIMBO; break;
            case C_MAPMODEL: newtype = MAPMODEL; break;
            case C_LADDER: newtype = LADDER; break;
            case C_SOUND: newtype = MAPSOUND; break;
            // not supported by the import:
            case C_CLIP:
            case C_PLCLIP:
            case C_DUMMYENT:
            default: return; 
        }

        extentity& e = *entities::newentity();
        entities::getents().add(&e);
        e.o = vec(ce.x * 4 + worldsize / 4, ce.y * 4 + worldsize / 4, ce.z * 4 + worldsize / 2);
        e.type = newtype;
        switch(e.type)
        {
            case LIGHT:
                e.attr1 = ce.attr1; // radius 
                e.attr1 *= 4;
                if (!ce.attr3 && !ce.attr4) { e.attr2 = e.attr3 = e.attr4 = ce.attr2; } // intensitiy e.g. white light
                else { e.attr2 = ce.attr2; e.attr3 = ce.attr3; e.attr4 = ce.attr4; } // individual r,g,b values
                break;

            case PLAYERSTART:
                e.attr1 = ((ce.attr1 / 10) + 270) % 360; // angle
                e.attr2 = ce.attr2; // team
                break;

            case CLIPS:
            case AMMO:
            case GRENADE:
            case HEALTH: 
            case HELMET: 
            case ARMOUR: 
            case AKIMBO:
                e.o.z = (float)(S(ce.x, ce.y)->floor + ce.attr1 / 5) * 4 + worldsize / 2; // elevation
                break;

            case MAPMODEL:
                e.o.z = (float)(S(ce.x, ce.y)->floor + ce.attr3 / 5) * 4 + worldsize / 2; // elevation
                e.attr1 = ce.attr2; // idx
                e.attr2 = ((ce.attr1 / 10) + 270) % 360; // yaw
                e.attr3 = ce.attr5 / 10; // pitch
                e.attr4 = ce.attr6; // roll
                e.attr5 = 25; // scale
                // todo: texture?
                break;

            case LADDER:
                e.attr1 = ce.attr1; // todo: support ladder entities
                break;

            case FLAG:
                e.attr1 = ((ce.attr1 / 10) + 270) % 360; // angle
                e.attr2 = ce.attr2; // red/blue
                break;

            case MAPSOUND:
                e.attr1 = ce.attr1; // sound idx
                e.attr2 = ce.attr2 * 4; // radius
                e.attr3 = ce.attr3; // todo: size - not supported yet
                e.attr3 = ce.attr4; // todo: volume - not supported yet
                break;

            default:
                e.attr1 = ce.attr1;
                e.attr2 = ce.attr2;
                e.attr3 = ce.attr3;
                e.attr4 = ce.attr4;
                e.attr5 = ce.attr5;
                break;
        }
    }

    cube &getcube(int x, int y, int z)
    {
        return lookupcube(ivec(x*4+worldsize/4, y*4+worldsize/4, z*4+worldsize/2), 4);
    }

    int neighbours(c_sqr &t)
    {
        o[0] = &t;
        o[1] = &t+1;
        o[2] = &t+ssize;
        o[3] = &t+ssize+1;
        int best = 0xFFFF;
        loopi(4) if(o[i]->vdelta<best) best = o[i]->vdelta;
        return best;
    }

    void preprocess_cubes()     // pull up heighfields to where they don't cross cube boundaries
    {
        for(;;)
        {
            bool changed = false;
            loop(x, ssize)
            {
                loop(y, ssize)
                {
                    c_sqr &t = world[x+y*ssize];
                    if(t.type==C_FHF || t.type==C_CHF)
                    {
                        int bottom = (neighbours(t)&(~3))+4;
                        loopj(4) if(o[j]->vdelta>bottom) { o[j]->vdelta = bottom; changed = true; }
                    }
                }
            }
            if(!changed) break;
        }
    }

    int getfloorceil(c_sqr &s, int &floor, int &ceil)
    {
        floor = s.floor;
        ceil = s.ceil;
        int cap = 0;
        switch(s.type)
        {
            case C_SOLID: floor = ceil; break;
            case C_FHF: floor -= (cap = neighbours(s)&(~3))/4; break;
            case C_CHF: ceil  += (cap = neighbours(s)&(~3))/4; break;
        }
        return cap;
    }

    void boundingbox()
    {
        x0 = y0 = ssize;
        x1 = y1 = 0;
        z0 = 128;
        z1 = -128;
        loop(x, ssize) loop(y, ssize)
        {
            c_sqr &t = world[x+y*ssize];
            if(t.type!=C_SOLID)
            {
                if(x<x0) x0 = x;
                if(y<y0) y0 = y;
                if(x>x1) x1 = x;
                if(y>y1) y1 = y;
                int floor, ceil;
                getfloorceil(t, floor, ceil);
                if(floor<z0) z0 = floor;
                if(ceil>z1) z1 = ceil;
            }
        }
    }

    void hf(int x, int y, int z, int side, int dir, int cap)
    {
        cube &c = getcube(x, y, z);
        loopi(2) loopj(2) edgeset(cubeedge(c, 2, i, j), side, dir*(o[(j<<1)+i]->vdelta-cap)*2+side*8);
    }

    bool cubesolidandoutsidefloorceil(int z, c_sqr *s) { return s->type==C_SOLID || z<s->floor || z>=s->ceil; }

    void createcorner(cube &c, int lstart, int lend, int rstart, int rend)
    {
        int ledge = edgemake(lstart, lend);
        int redge = edgemake(rstart, rend);
        cubeedge(c, 1, 0, 0) = ledge;
        cubeedge(c, 1, 1, 0) = ledge;
        cubeedge(c, 1, 0, 1) = redge;
        cubeedge(c, 1, 1, 1) = redge;
    }

    void create_cubes()
    {
        preprocess_cubes();
        boundingbox();
        lastremip = allocnodes;
        progress = 0;
        for(int x = x0-1; x<=x1+1; x++) for(int y = y0-1; y<=y1+1; y++)
        {
            c_sqr &s = world[x+y*ssize];
            int floor, ceil, cap = getfloorceil(s, floor, ceil);
            int prevcorner = -1; // ensure that all corners on the same z axis face same direction
           
            for(int z = z0-1; z<=z1+1; z++)
            {
                cube &c = getcube(x, y, z);
                
                // apply textures
                int walltex = s.type != C_SOLID && z < ceil ? s.wtex : s.utex;
                c.texture[O_LEFT] = c.texture[O_RIGHT] = c.texture[O_BACK] = c.texture[O_FRONT] = walltex;
                c.texture[O_BOTTOM] = s.ctex;
                c.texture[O_TOP] = s.ftex;

                if(s.type==C_CORNER)
                {
                    // determine if neighboring solid cubes exist outside floor ceil range
                    c_sqr *ts, *bs, *ls, *rs;
                    bool tc = cubesolidandoutsidefloorceil(z, ts = &s-ssize);
                    bool bc = cubesolidandoutsidefloorceil(z, bs = &s+ssize);
                    bool lc = cubesolidandoutsidefloorceil(z, ls = &s-1);
                    bool rc = cubesolidandoutsidefloorceil(z, rs = &s+1);

                    // determine if neighboring cubes are solid
                    bool tcs = ts->type == C_SOLID;
                    bool bcs = bs->type == C_SOLID;
                    bool lcs = ls->type == C_SOLID;
                    bool rcs = rs->type == C_SOLID;

                    // By default a corner surface is applies below floor level and above ceil level.
                    // An exception to this is having two neighboring solid cubes in which case the corner surface
                    // is drawn from floor to ceil instead.
                    int cornertype = CORNERTYPE_NONE;
                    if (tcs && lcs && !bcs && !rcs) cornertype = CORNERTYPE_BETWEEN_FLOOR_CEIL;
                    else if (tcs && !lcs && !bcs && rcs) cornertype = CORNERTYPE_BETWEEN_FLOOR_CEIL;
                    else if (!tcs && lcs && bcs && !rcs) cornertype = CORNERTYPE_BETWEEN_FLOOR_CEIL;
                    else if (!tcs && !lcs && bcs && rcs) cornertype = CORNERTYPE_BETWEEN_FLOOR_CEIL;
                    else cornertype = CORNERTYPE_OUTSIDE_FLOOR_CEIL;

                    if (tc && lc && !bc && !rc && (prevcorner == -1 || prevcorner == 0)) 
                    { 
                        // TOP LEFT
                        if(allowcorner(cornertype, z, floor, ceil)) { createcorner(c, 0, 8, 0, 0); prevcorner = 0; }
                        else setfaces(c, F_EMPTY);
                    }   
                    else if(tc && !lc && !bc && rc && (prevcorner == -1 || prevcorner == 1)) 
                    { 
                        // TOP RIGHT
                        if(allowcorner(cornertype, z, floor, ceil)) { createcorner(c, 0, 0, 0, 8); prevcorner = 1; }
                        else setfaces(c, F_EMPTY);
                    }   
                    else if(!tc && lc && bc && !rc && (prevcorner == -1 || prevcorner == 2)) 
                    {
                        // BOT LEFT
                        if(allowcorner(cornertype, z, floor, ceil)) { createcorner(c, 0, 8, 8, 8); prevcorner = 2; }
                        else setfaces(c, F_EMPTY);
                    }
                    else if(!tc && !lc && bc && rc && (prevcorner == -1 || prevcorner == 3)) 
                    { 
                        // BOT RIGHT        
                        if(allowcorner(cornertype, z, floor, ceil)) { createcorner(c, 8, 8, 0, 8); prevcorner = 3; }
                        else setfaces(c, F_EMPTY);
                    }                
                    else
                    {
                        if ((cornertype == CORNERTYPE_OUTSIDE_FLOOR_CEIL && z >= floor && z < ceil) || (!tc && !lc && !bc && !rc)) setfaces(c, F_EMPTY);

                        // fix texture on ground of a corner
                        if (ts->floor - 1 == z && bs->floor - 1 != z) { c.texture[O_TOP] = ts->ftex; }
                        else if (ts->floor - 1 != z && bs->floor - 1 == z) { c.texture[O_TOP] = bs->ftex; }
                        if (ts->ceil == z && bs->ceil != z) { c.texture[O_BOTTOM] = ts->ctex; }
                        else if (ts->ceil != z && bs->ceil == z) { c.texture[O_BOTTOM] = bs->ctex; }
                    }
                } 
                else if (z >= floor && z < ceil)
                {
                    setfaces(c, F_EMPTY);
                }
            }
            switch(s.type)
            {
                case C_FHF: hf(x, y, floor-1, 1, -1, cap); break;
                case C_CHF: hf(x, y, ceil, 0, 1, cap); break;
            }
            if(importcuberemip && (allocnodes - lastremip) * 8 > importcuberemip * 1024)
            {
                mpremip(true);
                lastremip = allocnodes;
            }
            if((progress++&0x7F)==0)
            {
                float bar = float((y1-y0+2)*(x-x0+1) + y-y0+1) / float((y1-y0+2)*(x1-x0+2));
                defformatstring(text, "creating cubes... %d%%", int(bar*100));
                renderprogress(bar, text);
            }
        }
    }

    bool allowcorner(int cornertype, int z, int floor, int ceil)
    {
        return ((cornertype == CORNERTYPE_BETWEEN_FLOOR_CEIL && z >= floor && z < ceil) || (cornertype == CORNERTYPE_OUTSIDE_FLOOR_CEIL && (z < floor || z >= ceil)));
    }

    int fixmapheadersize(int version, int headersize)   // we can't trust hdr.headersize for file versions < 10 (thx flow)
    {
        if (version < 4) return sizeof(c_header) - sizeof(int) * 16;
        else if (version == 7 || version == 8) return sizeof(c_header) + sizeof(char) * 128;  // mediareq
        else if (version < 10 || headersize < int(sizeof(c_header))) return sizeof(c_header);
        return headersize;
    }

    // headerextra stores additional data in a map file (support since format 10)
    // data can be persistent or oneway
    // the format and handling is explicitly designed to handle yet unknown header types to avoid further format version bumps

    struct headerextra
    {
        int len, flags;
        uchar* data;
        headerextra() : len(0), flags(0), data(NULL) {}
        headerextra(int l, int f, uchar* d) : len(l), flags(f), data(NULL) { if (d) { data = new uchar[len]; memcpy(data, d, len); } }
        ~headerextra() { DELETEA(data); }
        headerextra* duplicate() { return new headerextra(len, flags, data); }
    };

    vector<headerextra*> headerextras;

    void clearheaderextras() { loopvrev(headerextras) delete headerextras.remove(i); }

    void unpackheaderextra(uchar* buf, int len)  // break the extra data from the mapheader into its pieces
    {
        ucharbuf p(buf, len);
        while (1)
        {
            int len = getuint(p), flags = getuint(p);
            if (p.overread() || len > p.remaining() || len < 0 || flags < 0) break;
            headerextras.add(new headerextra(len, flags, p.subbuf(len).buf));
        }
    }

    void sqrdefault(c_sqr* s)
    {
        if (!s) return;
        s->r = s->g = s->b = 150;
        s->ftex = DEFAULT_FLOOR;
        s->ctex = DEFAULT_CEIL;
        s->wtex = s->utex = DEFAULT_WALL;
        s->type = C_SOLID;
        s->floor = 0;
        s->ceil = 16;
        s->vdelta = s->defer = s->tag = 0;
    }

    bool rldecodecubes(ucharbuf& f, c_sqr* s, int len, int version, bool silent, int cubicsize) // run-length decoding of a series of cubes (version is only relevant, if < 6)
    {
        c_sqr* t = NULL, * e = s + len;
        while (s < e)
        {
            int type = f.overread() ? -1 : f.get();
            switch (type)
            {
            case -1:
            {
                if (!silent) conoutf("while reading map at %d: unexpected end of file", int(cubicsize - (e - s)));
                f.forceoverread();
                silent = true;
                sqrdefault(s);
                break;
            }
            case 255:
            {
                if (!t) { f.forceoverread(); continue; }
                int n = f.get();
                loopi(n) memcpy(s++, t, sizeof(c_sqr));
                s--;
                break;
            }
            case 254: // only in MAPVERSION<=2
            {
                if (!t) { f.forceoverread(); continue; }
                memcpy(s, t, sizeof(c_sqr));
                f.get(); f.get();
                break;
            }
            case C_SOLID:
            {
                sqrdefault(s);                  // takes care of ftex, ctex, floor, ceil and tag
                s->type = C_SOLID;
                s->utex = s->wtex = f.get();
                s->vdelta = f.get();
                if (version <= 2) { f.get(); f.get(); }
                break;
            }
            case 253: // SOLID with all textures during editing (undo)
                type = C_SOLID;
            default:
            {
                if (type < 0 || type >= C_MAXTYPE)
                {
                    if (!silent) conoutf("while reading map at %d: type %d out of range", int(cubicsize - (e - s)), type);
                    f.forceoverread();
                    continue;
                }
                sqrdefault(s);
                s->type = type;
                s->floor = f.get();
                s->ceil = f.get();
                if (s->floor >= s->ceil) s->floor = s->ceil - 1;  // for pre 12_13
                s->wtex = f.get();
                s->ftex = f.get();
                s->ctex = f.get();
                if (version <= 2) { f.get(); f.get(); }
                s->vdelta = f.get();
                s->utex = (version >= 2) ? f.get() : s->wtex;
                s->tag = (version >= 5) ? f.get() : 0;
            }
            }
            s->defer = 0;
            t = s;
            s++;
        }
        return !f.overread();  // true: no problem
    }

    void processheaderextra(char *cfgname)
    {
        loopv(headerextras)
        {
            ucharbuf q(headerextras[i]->data, headerextras[i]->len);
            int type = headerextras[i]->flags & HX_TYPEMASK;
            switch (type)
            {
            case HX_EDITUNDO:
                break;

            case HX_CONFIG:
            {
                // AC format 10 has its whole config inside the header
                if (headerextras[i]->len <= 0 || headerextras[i]->data[headerextras[i]->len - 1] != '\0') // needs to have '\0' at the end, better check...
                {
                    conoutf("\f3malformed embedded config");
                    break;
                }

                // the mapmodel and texture commands from AC differs to cube2
                // therefore replace it so that we can provide backward compatibility
                char* cfg = replace((const char*)q.buf, "mapmodel ", "ac_mapmodel ");
                cfg = replace(cfg, "resetmapmodel", "");
                acmapmodels.setsize(0);
                cfg = replace(cfg, "texture ", "ac_texture ");
                cfg = replace(cfg, "texturereset", "");
                actextures.setsize(0);

                // execute the config with backward compatibility in place
                execute(cfg);
                
                // now remove the backward compatibility code from the script and save it
                cfg = replace(cfg, "ac_mapmodel ", "");
                cfg = replace(cfg, "ac_texture ", "");
                stream* f = openutf8file(path(cfgname, true), "w");
                if(f) 
                { 
                    f->write(cfg, strlen(cfg));

                    // apply new syntax for mapmodels and textures to config
                    f->printf("mapmodelreset\n");
                    loopv(acmapmodels) f->printf("mapmodel \"%s\"\n", acmapmodels[i].name);
                    f->printf("texturereset\n");
                    loopv(actextures)
                    {
                        float newscale = 2.0f / (actextures[i].scale > 0 ? actextures[i].scale : 1);
                        char *lastdot = strrchr(actextures[i].name, '.');
                        bool hasfileextension = (lastdot && lastdot > actextures[i].name);
                        string tex;
                        if(hasfileextension) 
                        {
                            size_t len = min(lastdot - actextures[i].name, MAXSTRLEN-1);
                            strncpy(tex, actextures[i].name, len);
                            tex[len] = '\0';
                        }
                        else strncpy(tex, actextures[i].name, MAXSTRLEN);
                        f->printf("texload \"%s\"; texscale %.1f; setshader \"stdworld\";\n", tex, newscale);

                        // uncomment line below to get oldschool texture look instead (only diffuse tex, no bumpmaps, etc.)
                        // this could be extended so that we automatically fallback to oldschool textures if the file at path ${tex}.tex does not exist
                        //f->printf("texture 0 \"%s\"; texscale %.1f\n", actextures[i].name, newscale);
                    }
                    delete f;
                }
                else conoutf("\f3could not write .cfg file");
                break;
            }

            case HX_MAPINFO:
            {
                string l, c;
                getstring(l, q, MAXSTRLEN - 1);
                getstring(c, q, MAXSTRLEN - 1);
                // TODO
                //setmapinfo(l, c);
                break;
            }
            case HX_MODEINFO:
            case HX_ARTIST:
            default:
                break;
            }

        }
    }

    void load_assaultcube_world(char *mname, int size)
    {
        // 12 is a sufficiently large map size to fit in legacy assaultcube maps
        if(!size) size = 12;

        int loadingstart = SDL_GetTicks();
        string pakname, cgzname;
        formatstring(pakname, "legacyformat/%s", mname);
        formatstring(cgzname, "media/map/%s.cgz", pakname);
        stream *f = opengzfile(path(cgzname), "rb");
        if(!f) { conoutf(CON_ERROR, "could not read assaultcube map %s", cgzname); return; }

        const int sizeof_header = sizeof(c_header), sizeof_baseheader = sizeof_header - sizeof(int) * 16;
        c_header tmp;
        memset(&tmp, 0, sizeof_header);
        if (f->read(&tmp, sizeof_baseheader) != sizeof_baseheader ||
            (strncmp(tmp.head, "ACMP", 4) != 0)) {
            conoutf("\f3while reading map: header malformatted (1)"); delete f; return;
        }
        lilswap(&tmp.version, 4); // version, headersize, sfactor, numents
        if (tmp.version != 10) { conoutf("\f3the map must comply to map format 10 (AssaultCube v1.3), please upgrade your map before importing"); delete f; return; }
        if (tmp.sfactor<SMALLEST_FACTOR || tmp.sfactor>LARGEST_FACTOR || tmp.numents > MAXENTITIES) { conoutf("\f3illegal map size"); delete f; return; }
        sfactor = tmp.sfactor;
        tmp.headersize = fixmapheadersize(tmp.version, tmp.headersize);
        int restofhead = min(tmp.headersize, sizeof_header) - sizeof_baseheader;
        if (f->read(&tmp.waterlevel, restofhead) != restofhead) { conoutf("\f3while reading map: header malformatted (2)"); delete f; return; }
        clearheaderextras();
        if (tmp.headersize > sizeof_header)
        {
            int extrasize = tmp.headersize - sizeof_header;
            if (tmp.version < 9) extrasize = 0;  // throw away mediareq...
            else if (extrasize > MAXHEADEREXTRA) extrasize = MAXHEADEREXTRA;
            if (extrasize)
            { // map file actually has extra header data that we want too preserve
                uchar* extrabuf = new uchar[extrasize];
                if (f->read(extrabuf, extrasize) != extrasize) { conoutf("\f3while reading map: header malformatted (3)"); delete f; delete[] extrabuf; return; }
                unpackheaderextra(extrabuf, extrasize);
                delete[] extrabuf;
            }
            f->seek(tmp.headersize, SEEK_SET);
        }
      
        if(!haslocalclients()) game::forceedit("");
        emptymap(size ? size : 12, true, NULL);

        freeocta(worldroot);
        worldroot = newcubes(F_SOLID);
        defformatstring(cs, "importing %s", cgzname);
        renderbackground(cs);

        string cfgname;
        formatstring(cfgname, "media/map/%s_imported.cfg", mname);
        processheaderextra(cfgname);

        c_header hdr = tmp;
        vector<c_persistent_entity> cents;
        loopi(hdr.numents)
        {
            c_persistent_entity e;
            f->read(&e, sizeof(c_persistent_entity));
            lilswap(&e.x, 4);
            if (i < MAXENTS) cents.add(e);
        }

        ssize = 1<<hdr.sfactor;
        int mipsize = ssize * ssize;
        int cubicsize = ssize * ssize;
        world = new c_sqr[mipsize];
        memset(world, 0, mipsize * sizeof(c_sqr));

        vector<uchar> rawcubes; // fetch whole file into buffer
        loopi(9)
        {
            ucharbuf q = rawcubes.reserve(cubicsize);
            q.len = f->read(q.buf, cubicsize);
            rawcubes.addbuf(q);
            if (q.len < cubicsize) break;
        }
        delete f;

        ucharbuf uf(rawcubes.getbuf(), rawcubes.length());
        rldecodecubes(uf, world, cubicsize, hdr.version, false, cubicsize);

        loopv(cents) create_ent(cents[i]);

        identflags &= ~IDF_OVERRIDDEN;
        create_cubes();
        mpremip(true);
        clearlights();
        allchanged();
        vector<extentity *> &ents = entities::getents();
        loopv(ents) if(ents[i]->type==ET_PLAYERSTART) dropenttofloor(ents[i]);
        entitiesinoctanodes();
        conoutf("read assaultcube map %s (%.1f seconds)", cgzname, (SDL_GetTicks()-loadingstart)/1000.0f);
        
        string newmapname;
        formatstring(newmapname, "%s_imported", mname);
        save_world(newmapname);

        game::changemap(newmapname);
    }
};

cubeloader* currentcubeloader = NULL;

void ac_texture(int* scale, char* name)
{
    if(!currentcubeloader) return;
    currentcubeloader->ac_texture(scale, name);
}

COMMAND(ac_texture, "is");

void ac_mapmodel(int* r, int* h, int* z, int* dummy, char* name)
{
    if (!currentcubeloader) return;
    currentcubeloader->ac_mapmodel(r, h, z, dummy, name);
}

COMMAND(ac_mapmodel, "iiiis");

void importassaultcube(char *name, int *size)
{ 
    if(multiplayer()) return;
    currentcubeloader = new cubeloader();
    currentcubeloader->load_assaultcube_world(name, *size);
    delete currentcubeloader;
    currentcubeloader = NULL;
}

COMMAND(importassaultcube, "si");