## C++ AI maze game 
![game2](https://user-images.githubusercontent.com/85022169/136157478-e09a9d3a-d189-45ea-83ba-5b5dfdd5da0e.jpg)
![game3](https://user-images.githubusercontent.com/85022169/136157733-0dae433b-6bfa-4f94-9147-cef549111086.jpg)
### The executable
Download the file maze.exe. While doing so a warning from the browser may appear, just select keep rather than discard. When trying to run the file, it might take some time for the computer to scan the file.
### The project
1. the main logic is implemented in standard C++ which is portable for Linux
2. there are a few APIs designed for button click function calls
3. the UI is done by Borland C++ Builder, because it's cleaner and easier, and the key point is that it support standard C++, as versus  Visual family
# Snippet

```C++
    int recur_search(Pos pt, int di, int depth) { // search and mark deadend alley
        depth--; // search depth limitation
        if (depth<0) return 2; // abort search

        vector<int> vd;
        Pos nextpos;
        int dt; // turn around direction
        int vsz,result;
        dt=(di+2)%4; // opposite direction, which is the coming direction
        for (int i=0;i<4;i++) { // to find out how many valid directions to go
            if (i==dt) continue; // do not search this
            if (hit_wall2(pt,i)) continue; // not a valid way to go
            vd.push_back(i); // store possible directions of search
        }

        vsz=vd.size();
        if (vsz==0) { // if this is a dead end
            deadendnote[pt.x][pt.y]=-1; // mark this pos
            (*map_info)[pt.x][pt.y].type=-1;
            return -1; // as dead end return value
        }

        for (int j=0;j<vd.size();j++) { // search further
            nextpos.x=pt.x+dir[vd[j]][0];
            nextpos.y=pt.y+dir[vd[j]][1];
            if (searched[nextpos.x][nextpos.y]) continue; // already searched by other branches

            result=recur_search(nextpos,vd[j],depth);
            if (result<0) { // if next pos a dead end
                if (vd.size()==1) { // and this is a tunnel
                    deadendnote[pt.x][pt.y]=-1; // make a note, backing out from a deadend 
                    (*map_info)[pt.x][pt.y].type=-1;
                    return -1; // report back to caller
                }
            }
            if (result>2) { // next pos is 4-way intersection
                if (vsz>2) { // current pos is also a 4-way intersection
                    return vsz; // an open space, maze is not complete
                }
            }
//          if (nextpos.x==startpos.x && nextpos.y==startpos.y) return vsz; // if circling around
        }
        return vsz; // not dead end, still open, no result
    }
