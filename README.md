## C++ AI maze game 
![game2](https://user-images.githubusercontent.com/85022169/136157478-e09a9d3a-d189-45ea-83ba-5b5dfdd5da0e.jpg)
![game3](https://user-images.githubusercontent.com/85022169/136157733-0dae433b-6bfa-4f94-9147-cef549111086.jpg)
### The executable
Download the file maze.exe. While doing so a warning from the browser may appear, just select keep rather than discard. When trying to run the file, it might take some time for the computer to scan the file.
### The project
1. the main logic is implemented in standard C++ which is portable for Linux
2. there are a few APIs designed for button click function calls
3. the UI is done by Borland C++ Builder, because it's cleaner and easier, and the key point is that it support standard C++, as versus  Visual family
## Snippet
### Maze generating
```C++
    int grow_blocks() { // each time grow 1 wall from each block
        int csz, idx, vsz, in_progress,m;
		Pos pt;
        pt.x=-1;
        pt.y=-1;
        Line line;

        if (map_ready) return 0;

        csz=blocks.size();
        for (int i=0;i<csz;i++) {
            vsz=blocks[i].vpt.size();
            if (vsz==0) continue;
            while (vsz) { // try to add a wall to the block if possible
                if (blocks[i].countdown) { // keeps growing the same branch
                    idx=vsz-1; // which is from the top of the vector
                } else {
                    blocks[i].countdown=(rand()%10)+3; // how long would the new branch be
                    idx = rand() % vsz; // from where does the new branch start
                }
      		m = get_neighboring_node(blocks[i].vpt[idx]);//m:0~3 direction
                pt.x=blocks[i].vpt[idx].x+dir[m][0];
                pt.y=blocks[i].vpt[idx].y+dir[m][1];
                if (m>=0) { // there is room to grow the block
	    		    grid[pt.x][pt.y] = 2;
    	    		blocks[i].vpt.push_back(pt); //growth record/prolific point
                    line.p1=blocks[i].vpt[idx];
                    line.p2=pt;
                    line.d=m;
                    blocks[i].qln.push(line); // for UI drawing
                    build_wall(line); // add to array walls[60][40][4]
                    blocks[i].countdown--;
                    break; // a new wall added
    	        } else { // no room to grow, try another starting point
                    blocks[i].vpt.erase(blocks[i].vpt.begin()+idx); // no more prolific
                    blocks[i].countdown=0; // grow as a new branch
                }
                vsz=blocks[i].vpt.size();
            }
        }

        in_progress=0;
        for (int i=0;i<csz;i++) {
            vsz=blocks[i].vpt.size();
            if (vsz) in_progress++;
        }
        if (in_progress==0) { // nothong to build this time
            maze_ready=true;
            map_ready=false;
            init_map();
        }
        return in_progress;
    }
```
### Explore dead end alleys
```C
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
```   
### Compare directions for approaching target
```C
    int recur_relay3(Pos pt, int di, int depth, Pos startpos,int nearest3, int &dist) {
        depth--; // search depth limitation
        if (depth<0) {
            return nearest3; // nearest3 is the distance of the..
        }                    // ..closest 3/4-way intersection to beacon
        vector<int> vd;
        Pos npt;
        int dt,ret,sml; // turn around direction
        int vsz,result,dis2ent;
        npt.x=pt.x+dir[di][0];
        npt.y=pt.y+dir[di][1];
        if ((*beacons)[target_beacon].p==npt) {
            if (depth>dist) { // initial dist is -99
                dist=depth; // dist is set for one time only, as depth is decreasing
            }         // the first time beacon found, store depth to dist
            return 1;
        }
//        if (npt.x==startpos.x && npt.y==startpos.y) {
//            return nearest3; // if circling around
//        }
        if ((*beacons)[target_beacon].entrance.p==npt) {
            if (depth>dist) { // if there are multiple branches found beacon..
                dist=depth; // they all have return value as 1..
            }             // we need to know which search branch has the..
            return 1;    // shortest approach to beacon, which is dist
        }
        if ((*map_info)[npt.x][npt.y].ways>=3) {
            dis2ent=abs((*beacons)[target_beacon].entrance.p.x-npt.x);
            dis2ent+=abs((*beacons)[target_beacon].entrance.p.y-npt.y);
            if (dis2ent<nearest3) {
                nearest3=dis2ent; // keep getting the smallest nearest3
            }
        }
        dt=(di+2)%4; // opposite direction, which is the coming direction
        for (int i=0;i<4;i++) { // to find out how many valid directions to go
            if (i==dt) continue; // do not search this
            if (not_road(npt,i)) continue; // not a valid way to go
            vd.push_back(i); // store possible directions of search
        }
        vsz=vd.size();
        sml=nearest3;
        for (int j=0;j<vsz;j++) { // search further
//          nextpos.x=pt.x+dir[vd[j]][0];
//          nextpos.y=pt.y+dir[vd[j]][1];
//          if (nextpos.x==startpos.x && nextpos.y==startpos.y) return nearest3; // if circling around
            ret=recur_relay3(npt,vd[j],depth,startpos,nearest3,dist);
            if (ret<sml) sml=ret; // compare return values from child branches
        }
        return sml; // return the smallest nearest3
    }
```
## Pseudo code - inheritance / virtual function / static member / pointer to parent
```C++
class Runner {
public:
    Info (*map_info)[60][40]; // a pointer to maze member
    virtual hit_wall(Pos,int); // intended to be overriden by child object
    static int dir[4][2]; // to be shared by all child classes
};
int Runner::dir[4][2]={-1,0,0,-1,1,0,0,1}; // initialize a static class member

class Player:public Runner { // inherit base class
public:
    vector<Runner> *ghosts; // undefined ghost number
    hit_wall(Pos,int);
};

class Maze {
public:
    Info map_info[60][40]; // to be shared in the game
    Player player; // there will be one player
    vector<Runner> ghosts; // uncertain number of ghosts

    Maze() {
        player.map_info=&map_info; // pass down the address to members
        player.ghosts=&ghosts; // pass down the address of ghost objects to player
    }
};
```
