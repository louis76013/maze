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
 bool explore_map() {
        //if (!maze_ready) return false;
        vector<int> vd;
        Pos pt;
        bool found; // found suitable starting point
        int result;
//        openspace=false;
        memset(deadendnote,0,sizeof(int)*60*40);
        for (int i=30-6;i<30+6;i++) { // select starting point
            for (int j=20-4;j<20+4;j++) {
                vd.resize(0);
                found=false;
                pt.x=i;
                pt.y=j;
                for (int di=0;di<4;di++) {
                    if (hit_wall2(pt,di)) continue;
                    vd.push_back(di);
                }
                if (vd.size()<3) continue; // not a 3-way intersection
//                if (vd.size()>3) {
//                    return false; // open space, map not ready
//                }
                found=true;
                break;
            }
            if (found) break;
        }
        // reset deadendnote before
        goodstarter=found;
        memset(searched,0,sizeof(int)*60*40);
        searched[pt.x][pt.y]=1;
        for (int i=0;i<vd.size();i++) {
            recur_search(pt,vd[i],120);
        }
        return true;
    }
