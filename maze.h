//----------------------
#include <windows.h>

#include <string>
#include <queue>
#include <vector>
#include <ctime>
#include <math.h>
#include "struct.h"

using namespace std;

struct Line {
    Pos p1,p2;
    int d; // direction {0,1,2,3}

};
struct Path {
    Pos p;
    int d;
};

struct Info {
    Pos p; // entrance pos
    int depth; // from entrance
    int d; // direction into alley from entrance
    int type; // 0: main road, -1: alley 1: entrance
    int ways; // intersection
    int steps; // steps from entrance
};

struct Beacon {
    Pos p;
    bool visited;
    bool on_main_road;
    int attempt; // how many times trying to approach
    Path entrance; // entrance to alley;
    vector<Path> path_in; //path from main road to beacon
    vector<Path> path_out; //path from beacon to main road
};


class Runner {
public:
    Pos p0,p; //p0: old, p: new
    int d; //direction
    int id; // player: 10 ghost: 1
    int deadendnote[60][40];
    int searched[60][40];

    bool deadendalley;
//    bool openspace;
    bool goodstarter;
    String info;
    int (*walls)[60][40][4];
    int (*deployment)[60][40];
    int (*map)[60][40];
    Info (*map_info)[60][40];

    bool *map_ready;
    static int dir[4][2];
    Runner() {
        id=1; // default ghost
        d=0;
        memset(deadendnote,0,sizeof(int)*60*40);
//        p0.x=0;
//        p0.y=0;
        p0=Pos(0,0);
        deadendalley=false;

    }

    bool main_road_check(Pos pt, int di) {
        int x,y;
        x=pt.x+dir[di][0];
        y=pt.y+dir[di][1];
        if (x<0 ||x>=60) return false;
        if (y<0 || y>=40) return false;
        if ((*map_info)[x][y].ways<2) return false; // not valid
        return true;
    }

    void protagonist() {
        id=10;
    }

    virtual bool hit_wall(int i) {
        int x,y;
        x=p.x+dir[i][0];
        y=p.y+dir[i][1];
        if (x<0 ||x>=60) return true;
        if (y<0 || y>=40) return true;
        if (id<10) if (deadendnote[x][y]<0) return true;
        if ((*walls)[p.x][p.y][i]) return true;
        return false;
    }

    virtual bool hit_wall2(Pos pt,int i) {
        int x,y;
        x=pt.x+dir[i][0];
        y=pt.y+dir[i][1];
        if (x<0 ||x>=60) return true;
        if (y<0 || y>=40) return true;

        if (deadendnote[x][y]<0) return true;
        if ((*walls)[pt.x][pt.y][i]) return true;

        return false;
    }

    int shortest_direction(vector<int> &vd, Pos &pt) { // dir. options, target
        vector<int> vs; //square sum
        int vsz,sum,ishortest;
        String str="";
        bool shortest;
        vsz=vd.size();
        for (int i=0;i<vsz;i++) {
            sum=0;
            sum+=pow(p.x+dir[vd[i]][0]-pt.x,2);
            sum+=pow(p.y+dir[vd[i]][1]-pt.y,2);
            vs.push_back(sum);
            str+=IntToStr(vd[i])+" "+IntToStr(sum)+" ";
        }
        ishortest=vd.back(); // pre-set shortest as last
        for (int i=0;i<vsz;i++) {
            shortest=true;
            for (int j=i+1;j<vsz;j++) {
                if (vs[i]>vs[j]) {
                    shortest=false;
                    break;
                }
            }
            if (shortest) {
                str+=IntToStr(vd[i]);
                info=str;
                ishortest=i;
                break;
                //return vd[i];
            }
        }
        str+=IntToStr(vd.back());
        info=str;

        if (id>=10) return vd[ishortest]; // for suvivor, go with shortest

        if (rand()%10 >=5) { // probability to go with the shortest dir.
            return vd[ishortest];
        } else {
            return vd[rand()%vsz]; // randomly pick a dir.
        }
    }

    bool track(Pos pt) { // target pos; pt to be tracked
        vector<int> vd;
        int dt; // turn around direction
        int nd; // new direction
        dt=(d+2)%4;
        for (int di=0;di<4;di++) {
            if (di==dt) continue;
            if (hit_wall(di)) continue;
            vd.push_back(di);
        }

        switch (vd.size()) {
        case 0:
            nd=dt;
            deadendalley=true;
            break;
        case 1:
            nd=vd.front();
            break;
        default:
            if (deadendalley) {
                deadendnote[p0.x][p0.y]=-1;
                deadendalley=false;
            }
            nd=shortest_direction(vd,pt);
            break;
        }
        d=nd;
        p0.x=p.x;
        p0.y=p.y;
        p.x=p.x+dir[d][0];
        p.y=p.y+dir[d][1];
        return true;
    }

    Pos pos() {
        return p;
    }

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
            if (searched[nextpos.x][nextpos.y]) continue; // already searched

            result=recur_search(nextpos,vd[j],depth);
            if (result<0) { // if next pos a dead end
                if (vd.size()==1) { // and this is a tunnel
                    deadendnote[pt.x][pt.y]=-1; // mark the current pos as dead end
                    (*map_info)[pt.x][pt.y].type=-1;
                    return -1; // report back to caller
                }
            }
            if (result>2) { // next pos is open space
                if (vsz>2) { //current pos is open space
                    return vsz; // consecutive open space
                }
            }
//            if (nextpos.x==startpos.x && nextpos.y==startpos.y) return vsz; // if circling around
        }
        return vsz; // not dead end, still open, no result
    }

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

};
int Runner::dir[4][2]={-1,0,0,-1,1,0,0,1};


class Player:public Runner {
public:
    int score[4];
    int dsort[4];//
    bool moved; // if pinched by ghosts then false
    bool go_on_beacon_path;

    int target_beacon;
    queue<Path> planned_path;
    vector<Runner> *ghosts;
    vector<Beacon> *beacons;

    bool not_road(Pos pt,int i) {
        int x,y;
        x=pt.x+dir[i][0];
        y=pt.y+dir[i][1];
        if (x<0 ||x>=60) return true;
        if (y<0 || y>=40) return true;
//        if (id<10) if (deadendnote[x][y]<0) return true;
        if ((*walls)[pt.x][pt.y][i]) return true;
        if ((*map_info)[pt.x][pt.y].type<0) return true;
        return false;

    }
    bool hit_wall(Pos pt, int i) {
        int x,y;
        x=pt.x+dir[i][0];
        y=pt.y+dir[i][1];
        if (x<0 ||x>=60) return true;
        if (y<0 || y>=40) return true;
//        if (id<10) if (deadendnote[x][y]<0) return true;
        if ((*walls)[pt.x][pt.y][i]) return true;
//        if (map_info[][].type<0) return true;
        return false;
    }
/*
    bool hit_wall(Pos pt,int i) {
        int x,y;
        x=pt.x+dir[i][0];
        y=pt.y+dir[i][1];
        if (x<0 ||x>=60) return true;
        if (y<0 || y>=40) return true;
//        if (deadendnote[x][y]<0) return true;
        if ((*walls)[pt.x][pt.y][i]) return true;
        return false;
    }
*/


    void nearest_entrance() {
        vector<int> dist,bid;
        bool shortest;
        int dx,dy;
        for (int i=0;i<(*beacons).size();i++) {
            if ((*beacons)[i].visited) continue;
//            if (!(*beacons)[i].on_main_road) continue;

            bid.push_back(i);
        }
        if (bid.size()==0) return;
        for (int i=0;i<bid.size();i++) {
            dx=(*beacons)[bid[i]].entrance.p.x-p.x;
            dy=(*beacons)[bid[i]].entrance.p.y-p.y;
            dist.push_back(abs(dx)+abs(dy));
        }
        for (int i=0;i<bid.size()-1;i++) {
            shortest=true; // assume dist[i] shortest
            for (int j=i+1;j<bid.size();j++) {
                if (dist[i]>dist[j]) {
                    shortest=false;
                    break;
                }
            }
            if (shortest) {
                target_beacon=bid[i];
                return ;
            }
        }
        target_beacon=bid.back();
        return;
    }

    int target_direction(vector<int> &vd) {
        vector<int> dist;
        int x,y,dx,dy,temp;
        int dim[5][2];
        int dimxy[5][2];

        bool shortest;
        int test;
        for (int di=0;di<vd.size();di++) {
            x=p.x+dir[vd[di]][0];
            y=p.y+dir[vd[di]][1];
            dx=(*beacons)[target_beacon].entrance.p.x-x;
            dy=(*beacons)[target_beacon].entrance.p.y-y;
            dist.push_back(pow(dx,2)+pow(dy,2));
        }

         for (int i=0;i<vd.size()-1;i++) {
            shortest=true; // assume dist[i] shortest
            for (int j=i+1;j<vd.size();j++) {
                if (dist[i]>dist[j]) {
                    shortest=false;
                    break;
                }
            }
            if (shortest) {
                return vd[i];
            }
        }

        return vd.back();
    }

    int ghost_distance(Pos pt,int cnt3way) { // 0:no ghost -1:here N:distance into alley
        int gsz;
        Pos gpt,ept;
        gsz=(*ghosts).size();

        for (int i=0;i<gsz;i++) {
            //gpt.x=(*ghosts)[i].p.x;
            //gpt.y=(*ghosts)[i].p.y;
            gpt=(*ghosts)[i].p;
            if ((*map_info)[gpt.x][gpt.y].type>=0){ //ghosst on main road
                //if (pt.x==gpt.x && pt.y==gpt.y) { // ghost here
                if (pt==gpt) { // ghost here
                    return -1;
                }

            } else {
                //ept.x=(*map_info)[gpt.x][gpt.y].p.x;
                //ept.y=(*map_info)[gpt.x][gpt.y].p.y;
                ept=(*map_info)[gpt.x][gpt.y].p;
//                if (pt.x==ept.x && pt.y==ept.y) { // ghost alley entrance here
                if (pt==ept) { // ghost alley entrance here

                    return (*map_info)[gpt.x][gpt.y].depth;
                }

            }
        }
        return 0;
    }

    int score_moves(int di,int di0) {
        int x,y,gx,gy,gsz;
        Pos pt,pg;
        gsz=(*ghosts).size();
        x=p.x+dir[di][0];
        y=p.y+dir[di][1];
        pt=Pos(x,y);
        for (int j=0;j<gsz;j++) {
            if ((*ghosts)[j].p==pt) {
                return -100;
            }
        }

        for (int i=0;i<gsz;i++) {
            gx=(*ghosts)[i].p.x;
            gy=(*ghosts)[i].p.y;

            for (int k=0;k<4;k++) {
                if ((*ghosts)[i].hit_wall(k)) continue;
                if (gx+dir[k][0]==x && gy+dir[k][1]==y) {
                     return -50;

                }
            }
        }

        if (di!=di0) return 100;
        else return 50;
    }

    int sort_score() {
        int n,di,no,dist[4];
        int scr[5]={100,50,-50,-100,-200};
        int idx=0;
        int dis2ent[4];
        bool good;
        for (int k=0;k<5;k++) {
            for (int i=0;i<4;i++) {
                if (score[i]==scr[k]) {
                    dsort[idx++]=i;
                }
            }
        }
        n=1;
        for (int i=0;i<4;i++) {
            if (score[dsort[i]]!=score[dsort[0]]) {
                n=i;
                break;
            }
        }

        if(score[dsort[0]]==100 && n>1) {
            memset(dist,-99,sizeof(int)*4);
            for (int i=0;i<n;i++) {
                dis2ent[i]=recur_relay3(p,dsort[i],40,p,200,dist[i]);
                if (dis2ent[i]==1) {
                    no=0;
                }
            }

            if ((dis2ent[0]==dis2ent[1])) {
                if (dist[0]>dist[1]) {
                    di=dsort[0];
                } else {
                    di=dsort[1];
                }
            } else {

                for (int i=0;i<n-1;i++) {
                    good=true;
                    for (int j=i+1;j<n;j++) {
                        if (dis2ent[i]>dis2ent[j]){
                            good=false;
                            break;
                        }
                    }
                    if (good) {
                        di=dsort[i];
                        break;
                    }
                }
                if (!good) {
                    di=dsort[n-1];
                }
            }

        } else {
            di=dsort[rand()%n];
        }
        return di;
    }

    bool recur_find_ghosts(Pos pt,int di,int depth) {
        depth--;
        if (depth<0) return false;
        vector<int> vd;
        Pos npt;
        int dt; // turn around direction
        int vsz;

        npt.x=pt.x+dir[di][0];
        npt.y=pt.y+dir[di][1];

        if ((*map)[npt.x][npt.y]>=1000) return true;

        dt=(di+2)%4; // opposite direction, which is the coming direction
        for (int i=0;i<4;i++) { // to find out how many valid directions to go
            if (i==dt) continue; // do not search this
            if (hit_wall2(npt,i)) continue; // not a valid way to go
            vd.push_back(i); // store possible directions of search
        }
        vsz=vd.size();
        for (int j=0;j<vsz;j++) { // search further
            if (recur_find_ghosts(npt,vd[j],depth)) {
                return true;
            }
        }
        return false;
    }

    bool action_to_beacon() {
        int x,y,depth,bcnt=0;
        x=(*beacons)[target_beacon].entrance.p.x;
        y=(*beacons)[target_beacon].entrance.p.y;

        if (p.x!=x || p.y!=y) return false;
        if ((*beacons)[target_beacon].on_main_road) return false;
        (*beacons)[target_beacon].attempt++;

        for(int i=0;i<5;i++) {
            if (!(*beacons)[i].visited) {
                bcnt++;
            }
        }
        if (bcnt>1) {
            depth= (*beacons)[target_beacon].path_in.size();
            depth+=(*beacons)[target_beacon].path_out.size();
        } else {
            depth=0;
        }
        for (int i=0;i<4;i++) {
            if (i==(*beacons)[target_beacon].path_in[0].d) {
                continue;
            }
            if (hit_wall(p,i)) continue;
            if (recur_find_ghosts(p,i,depth+1)) return false;
        }
        //if ((*beacons)[target_beacon].attempt>=2) return -1;
        return true;
    }
    void prepare_path_to_beacon() {
        int lenin,lenout,lenarr;
        Path path;
        int arr[30][3];

        lenin=(*beacons)[target_beacon].path_in.size();
        for (int i=0;i<lenin;i++) {
            path.p.x=(*beacons)[target_beacon].path_in[i].p.x;
            path.p.y=(*beacons)[target_beacon].path_in[i].p.y;
            path.d=(*beacons)[target_beacon].path_in[i].d;
            //to_beacon.push_back(path);
            planned_path.push(path);
        }
        lenout=(*beacons)[target_beacon].path_out.size();
        for (int i=0;i<lenout;i++) {
            path.p.x=(*beacons)[target_beacon].path_out[i].p.x;
            path.p.y=(*beacons)[target_beacon].path_out[i].p.y;
            path.d=(*beacons)[target_beacon].path_out[i].d;
            //to_beacon.push_back(path);
            planned_path.push(path);

        }
        lenarr=planned_path.size();
        go_on_beacon_path=true;

    }
    void prepare_path_to_escape() {
        int steps,x,y;
        Pos pt;
        Path path;
        steps=(*map_info)[p.x][p.y].steps;
        pt=p;
        for (int i=0;i<steps;i++) {
            for (int k=0;k<4;k++) {
                if (hit_wall(pt,k)) continue; //
                x=pt.x+dir[k][0];
                y=pt.y+dir[k][1];
                if ((*map_info)[x][y].steps==(*map_info)[pt.x][pt.y].steps-1) {
                    pt=Pos(x,y);
                    path.p=pt;
                    path.d=k;
                    planned_path.push(path);
                    break;
                }
            }
        }
    }
    void change_target_beacon() {
        for (int i=0;i<5;i++) {
            if ((*beacons)[i].visited) continue;
            if  (i==target_beacon) continue;
            target_beacon=i;
            return;
        }
    }

    bool ai_move() {
        int x,y,di,bwd,dr,xyz,dii;
        vector<int> vd,vs;
        int avd[4],avcd[4];
        Pos pt;
        Path path;
        int dd,strange;
        vector<int> vcd;
        bool act;
        int no,see;
        if (!(*map_ready)) return false;
        if (p.x<0 || p.y<0) return false;
        memset(deadendnote,0,sizeof(int)*60*40);


        if (planned_path.empty()) {
            if ((*map_info)[p.x][p.y].type<0) {
                prepare_path_to_escape();
            } else {
                if (action_to_beacon()) {
                    prepare_path_to_beacon();
                } else if ((*beacons)[target_beacon].attempt>=2) {
                    change_target_beacon();
                }
            }
        }
        if (!planned_path.empty()) {
            see=0;
            path=planned_path.front();
            if ((*map)[path.p.x][path.p.y]>=1000) {
                moved=false;
                return false;
            }
            p0.x=p.x;
            p0.y=p.y;
            p.x=path.p.x;
            p.y=path.p.y;
            d=path.d;
            planned_path.pop();
            moved=true;
            return true;

        }

        dr=(d+2)%4;
        for (int i=0;i<4;i++) {
//            if (hit_wall(i)) {
            if (not_road(p,i)) {

                score[i]=-200;
                continue;
            }
            memset(searched,0,sizeof(int)*60*40);
            recur_search(p,i,20);
            if (i==dr) {
                bwd=score_moves(i,dr);
                score[i]=bwd;
//                dr=i;
                continue;
            }
            pt.x=p.x+dir[i][0];
            pt.y=p.y+dir[i][1];

            if ((*map_info)[pt.x][pt.y].type<0) {
                score[i]=-200;
                continue;
            }
            vd.push_back(i);
            score[i]=score_moves(i,dr);
            vcd.push_back(score[i]);
        }
        if (dr>3) {
            xyz=0;
        }

        di=sort_score();

        x=p.x+dir[di][0];
        y=p.y+dir[di][1];

        if ((*map)[x][y]>=1000) {
            moved=false;
            return false;
        }

        moved=true;
        p0.x=p.x;
        p0.y=p.y;
        p.x=x;
        p.y=y;

        d=di;
        return true;
    }

    bool move(int di) {
        int x,y;
        if (p.x<0 || p.y<0) return false;
        if (di<0 || di>=4) return false;
        if (hit_wall(p,di)) return false;

        d=di;
        x=p.x+dir[di][0];
        y=p.y+dir[di][1];
        if ((*map)[x][y]/1000) { //ghost here
            return false;
        }
        memset(deadendnote,0,sizeof(int)*60*40);
        for (int i=0;i<4;i++) {
            if (not_road(p,i)) {
                continue;
            }
            memset(searched,0,sizeof(int)*60*40);
            recur_search(p,i,20);
        }
        p0.x=p.x;
        p0.y=p.y;
        p.x=x;
        p.y=y;
        return true;
    }
    int recur_relay3(Pos pt, int di, int depth, Pos startpos,int nearest3, int &dist) {
        depth--; // search depth limitation
        if (depth<0) {
            return nearest3; // end of search
        }
        vector<int> vd;
        Pos npt;
        int dt,ret,sml; // turn around direction
        int vsz,result,dis2ent;

        npt.x=pt.x+dir[di][0];
        npt.y=pt.y+dir[di][1];

        if ((*beacons)[target_beacon].p.x==npt.x) {
            if ((*beacons)[target_beacon].p.y==npt.y) {
                if (depth>dist) {
                    dist=depth;
                }
                return 1;
            }
        }
//        if (npt.x==startpos.x && npt.y==startpos.y) {
//            return nearest3; // if circling around
//        }

        if ((*beacons)[target_beacon].entrance.p.x==npt.x) {
            if ((*beacons)[target_beacon].entrance.p.y==npt.y) {
                if (depth>dist) {
                    dist=depth;
                }
                return 1;
            }
        }

        if ((*map_info)[npt.x][npt.y].ways>=3) {
            dis2ent=abs((*beacons)[target_beacon].entrance.p.x-npt.x);
            dis2ent+=abs((*beacons)[target_beacon].entrance.p.y-npt.y);
            if (dis2ent<nearest3) {
                nearest3=dis2ent;
                //dist=depth;
            }
        }

        dt=(di+2)%4; // opposite direction, which is the coming direction
        for (int i=0;i<4;i++) { // to find out how many valid directions to go
            if (i==dt) continue; // do not search this
//            if (hit_wall2(npt,i)) continue; // not a valid way to go
            if (not_road(npt,i)) continue; // not a valid way to go

            vd.push_back(i); // store possible directions of search
        }

        vsz=vd.size();

        sml=nearest3;

        for (int j=0;j<vsz;j++) { // search further
//            nextpos.x=pt.x+dir[vd[j]][0];
//            nextpos.y=pt.y+dir[vd[j]][1];
//            if (nextpos.x==startpos.x && nextpos.y==startpos.y) return nearest3; // if circling around

            ret=recur_relay3(npt,vd[j],depth,startpos,nearest3,dist);
//            if (ret==1) {
//                return 1;
//            }
            if (ret<sml) sml=ret;

        }
        return sml; //
    }
};

struct Block {
    vector<Pos> vpt;
    queue<Line> qln;
    int countdown;
};

typedef std::vector< std::vector<Pos> > vecArr;

class Maze {
public:
    int grid[61][41];
    int walls[60][40][4];
    int map[60][40];
    int deployment[60][40];
    Info map_info[60][40];
    int map_depth[60][40];
    int searched[60][40];
    int X, Y;
    int N;

    bool guide;
    bool radar;
    bool freeway;
    bool GameOver;
    bool maze_ready;
    bool map_ready;

//  int dir[4][2] = {-1,0,0,-1,1,0,0,1}; // not supported old version
	int dir[4][2];

    int countdown;
	queue<Line> ql;

	vector<Pos> v0;
    Player bot;
    Player player;
    vector<Runner> ghosts;
    vecArr vecP;
    vector<Block> blocks;
    vector<Beacon> beacons;

	Maze() {
        X=60;Y=40;
        init_dir();
//		srand(time(NULL));  // Initialize random number generator.
        srand(time(0));  // Initialize random number generator.
        reset();
        clear_runners();
        maze_ready=false;
        map_ready=false;
	}

    void init_dir() {
        int dir0[4][2] = {-1,0,0,-1,1,0,0,1};
        memcpy(dir,dir0,sizeof(int)*4*2);
    }

    void set_guide(bool b) {
        guide=b;
    }

    bool is_game_over() {
        int bsz;
        int gsz;
        bool allvisited=true;
        Pos pt;
        bsz=beacons.size();
        for (int i=0;i<bsz;i++) {
            if (!beacons[i].visited) {
                allvisited=false;
                break;
            }
        }
        if (allvisited) return true;
        gsz=ghosts.size();
        if (gsz==0) return false;
        for (int i=0;i<gsz;i++) {
            pt=ghosts[i].pos();
            if (pt.x==player.pos().x && pt.y==player.pos().y) {
                return true;
            }
        }
        return false;
    }

    void add_ghost(Pos pt) {
        int gsz;
        gsz=ghosts.size();
        ghosts.resize(gsz+1);
        ghosts[gsz].walls=&walls;
        ghosts[gsz].deployment=&deployment;
        ghosts[gsz].p.x=pt.x;
        ghosts[gsz].p.y=pt.y;
        ghosts[gsz].d=0;
        return;
    }

    void deploy_runners() {
        int bsz,gsz;
        Pos pt;
        vector<Pos> vpt;
        bool good;

        ghosts.resize(0);

        for (int i=0;i<beacons.size();i++) {
            pt.x=beacons[i].p.x;
            pt.y=beacons[i].p.y;
            vpt.push_back(pt);
        }
        for (int i=0;i<N;i++) { // last one for Survivor
            while (1) {
                pt.x=rand()%60;
                pt.y=rand()%40;
                good=true;
                for (int j=0; j<vpt.size();j++) { // keep spacing from deployed
                    if (abs(pt.x-vpt[j].x)+abs(pt.y-vpt[j].y) <10) {
                        good=false;
                        break;
                    }
                }
                if (good) {
                    break;
                }
            }
            add_ghost(pt);
            vpt.push_back(pt);
        }
        while(1) {
            pt.x=rand()%60;
            pt.y=rand()%40;
            good=true;
            for (int j=0; j<vpt.size();j++) { // keep spacing from deployed
                if (abs(pt.x-vpt[j].x)+abs(pt.y-vpt[j].y) <10) {
                    good=false;
                    break;
                }
            }
            if (!good) {
                continue;
            }
            if (map_info[pt.x][pt.y].type<0) {
                good=false;
                continue;
            }
            player.p.x=pt.x;
            player.p.y=pt.y;
            for(int k=0;k<4;k++) {
                if (!hit_wall(pt,k)) {
                    player.d=(k+2)%4;
                    break;
                }
            }
            break;
        }
        player.planned_path=queue<Path> ();
        for (int i=0;i<5;i++) beacons[i].attempt=0;
        player.nearest_entrance();
        memset(player.deadendnote,0,sizeof(int)*60*40);
    }

    bool init_map() {
        int test;
        if (!maze_ready) return false;
        //bot=Player();
        memset(map_info,0,sizeof(Info)*60*40);
        bot.map_info=&map_info;
        bot.explore_map();

        init_map_info();
        init_beacons();
        deploy_runners(); // do this after init_beacons()
        map_ready=true;
        return true;
    }

/*
    void init_dir() {
        int z;

        for (int i=0; i<4; i++) {
            z=(i>=2)?1:-1;
            dir[i][0]=(1-(i%2))*z;
            dir[i][1]=(i%2)*z;
        }
    }
*/

	int get_neighboring_node(Pos pt) {
		int px, py, n;
		Pos node; // node of 4 directions
		vector<Pos> vt; // store valid directions
        vector<int> vi; // store valid directions

		for (int i = 0; i < 4; i++) { // top, down, left, right
			px = pt.x + dir[i][0];
			py = pt.y + dir[i][1];
			if (px <= 0 || px >= X) {
				continue;
			}
			if (py <= 0 || py >= Y) {
				continue;
			}
			if (grid[px][py]) {
				continue;
			}
            vi.push_back(i);
		}
		if (vi.size() == 0) {
            return -1;
		}
		n = rand() % vi.size(); // choose one valid
		return vi[n];
	}

    void update_beacons() {
        int bsz;
        Pos pt;
        bsz=beacons.size();
        if (bsz==0) return;
        for (int i=0;i<bsz;i++) {
            pt.x=beacons[i].p.x;
            pt.y=beacons[i].p.y;

            if (player.pos().x==pt.x && player.pos().y==pt.y) {
                //beacons.erase(beacons.begin()+i);
                beacons[i].visited=true;
                player.nearest_entrance();
            }
        }
    }

    int grow_blocks() {
        int csz, idx, vsz, working,m;
		Pos pt;
        pt.x=-1;
        pt.y=-1;
        Line line;

        if (map_ready) return 0;

        csz=blocks.size();
        for (int i=0;i<csz;i++) {
            vsz=blocks[i].vpt.size();
            if (vsz==0) continue;
            while (vsz) {
                if (blocks[i].countdown) {
                    idx=vsz-1;
                } else {
                    blocks[i].countdown=(rand()%10)+3;
                    idx = rand() % vsz;
                }
        		m = get_neighboring_node(blocks[i].vpt[idx]);
                pt.x=blocks[i].vpt[idx].x+dir[m][0];
                pt.y=blocks[i].vpt[idx].y+dir[m][1];
                if (m>=0) {
	    		    grid[pt.x][pt.y] = 2;
    	    		blocks[i].vpt.push_back(pt);
                    line.p1=blocks[i].vpt[idx];
                    line.p2=pt;
                    line.d=m;
                    blocks[i].qln.push(line);
                    build_wall(line);
                    blocks[i].countdown--;
                    break;
    	        } else {
                    blocks[i].vpt.erase(blocks[i].vpt.begin()+idx);
                    blocks[i].countdown=0;
                }
                vsz=blocks[i].vpt.size();
            }
        }

        working=0;
        for (int i=0;i<csz;i++) {
            vsz=blocks[i].vpt.size();
            if (vsz) working++;
        }
        if (working==0) { // nothong to build this time
            maze_ready=true;
            map_ready=false;
            init_map();
        }

        return working;
    }

    bool is_maze_ready() {
        return maze_ready;
    }

    bool is_map_ready() {
        return map_ready;
    }

    //------------------------------------------
    void set_freeway(bool set) {
        freeway=set;
    }
    void set_radar(bool b) {
        radar=b;
    }
    void blue_print() {
        int csz;
        Pos p;
        // for edge
        blocks.resize(0);
        int r, dist=-7; //starting point
        while (!freeway) {
            r=rand()%10+10;
            if (dist+r==X || dist+r==X+Y || dist+r==X+X+Y || dist+r==X+X+Y+Y) {
                continue;
            }
            dist+=r;
            if (dist>X+X+Y+Y) break;
            csz=blocks.size();
            blocks.resize(csz+1);

            if (dist<X) {
                p.x=dist;
                p.y=0;
                blocks[csz].vpt.push_back(p);
            } else if (dist<X+Y) {
                p.x=X;
                p.y=dist-X;
                blocks[csz].vpt.push_back(p);
            } else if (dist<X+X+Y) {
                p.x=X+X+Y-dist;
                p.y=Y;
                blocks[csz].vpt.push_back(p);
            } else {
                p.x=0;
                p.y=X+X+Y+Y-dist;
                blocks[csz].vpt.push_back(p);
            }
        }
        // for center area
        int spacing; // x+y distance > 10
        int trialerror=0; // 5 times consecutively to stop
        int x,y,cx,cy;
        bool success;
        while (trialerror<300) {
            x=rand()%(X-1)+1;
            y=rand()%(Y-1)+1;
            if (grid[x][y]) continue;
            success=true;
            csz=blocks.size();
            for (int i=0;i<csz;i++) {
                cx=blocks[i].vpt[0].x;
                cy=blocks[i].vpt[0].y;
                spacing=pow(x-cx,2)+pow(y-cy,2);
                if (spacing<30) {
                    success=false;
                    break;
                }
            }
            if (success) {
                blocks.resize(csz+1);
                p.x=x;
                p.y=y;
                blocks[csz].vpt.push_back(p);
                grid[x][y]=2;
                trialerror=0;
            } else trialerror++;
        }
    }

    void build_wall(Line ln) {
        switch(ln.d) {
        case 0:
            walls[ln.p1.x-1][ln.p1.y][1]=1;
            walls[ln.p1.x-1][ln.p1.y-1][3]=1;
            break;
        case 1:
            walls[ln.p1.x][ln.p1.y-1][0]=1;
            walls[ln.p1.x-1][ln.p1.y-1][2]=1;
            break;
        case 2:
            walls[ln.p1.x][ln.p1.y][1]=1;
            walls[ln.p1.x][ln.p1.y-1][3]=1;
            break;
        case 3:
            walls[ln.p1.x][ln.p1.y][0]=1;
            walls[ln.p1.x-1][ln.p1.y][2]=1;
            break;
        }
    }

    void update_map() {  // explored, beacon, ghost, player
        int bsz,gsz;
        memset(map,0,sizeof(int)*60*40);

        for (int i=0;i<60;i++) {
            for (int j=0;j<40;j++) {
                if (guide) {
                    if (map_info[i][j].type<0) {
                        map[i][j]+=1;
                    }
                } else if (radar) {
                    if (player.deadendnote[i][j]<0) {
                        map[i][j]+=1;
                    }
                }
            }
        }
        bsz=beacons.size();
        for (int i=0;i<bsz;i++) {
            if (!beacons[i].visited) {
                map[beacons[i].p.x][beacons[i].p.y]+=10;
            }
        }
        gsz=ghosts.size();
        for (int i=0;i<gsz;i++) {
            map[ghosts[i].p.x][ghosts[i].p.y]+=1000;
        }
        map[player.p.x][player.p.y]+=100;
    }

     bool hit_wall(Pos pt,int di) {
        int x,y;
        x=pt.x+dir[di][0];
        y=pt.y+dir[di][1];
        if (x<0 ||x>=60) return true;
        if (y<0 || y>=40) return true;
        if (walls[pt.x][pt.y][di]) return true;
        return false;
    }

    int recur_beacons(Pos pt, int di,int bn) { // log path from beacon to entrance
        Pos pnow,pb;
        Path path;
        vector<int> vd;
        int vsz;
        pnow.x=pt.x+dir[di][0];
        pnow.y=pt.y+dir[di][1];
        pb.x=beacons[bn].p.x;
        pb.y=beacons[bn].p.y;
        if (pnow.x==pb.x && pnow.y==pb.y) {
            path.d=(di+2)%4;
            path.p.x=pt.x;
            path.p.y=pt.y;

            beacons[bn].path_out.push_back(path);
            return 1;
        }

        for (int i=0;i<4;i++) { // to find out how many valid directions to go
            if (i==(di+2)%4) continue; // do not search this
            if (hit_wall(pnow,i)) continue; // not a valid way to go
            vd.push_back(i); // store possible directions of search
        }
        vsz=vd.size();
        for (int j=0;j<vsz;j++) { // search further
            if (recur_beacons(pnow,vd[j], bn)==1) {
                path.d=(vd[j]+2)%4;
                path.p.x=pt.x;
                path.p.y=pt.y;
                beacons[bn].path_out.push_back(path);
                return 1;
            }
        }
        return 0;
    }

    void init_beacon_paths() {
        int bsz,di,psz;
        Pos pt,pb;
        Path path;
        bsz=beacons.size();
        for (int i=0;i<bsz;i++) {
            if (beacons[i].on_main_road) continue;
            beacons[i].path_out.resize(0); // clear path_out
            pt.x=beacons[i].entrance.p.x;
            pt.y=beacons[i].entrance.p.y;
            di=beacons[i].entrance.d;
            pb.x=beacons[i].p.x;
            pb.y=beacons[i].p.y;
            recur_beacons(pt,di,i); // stop if hit beacon
        }
        for (int i=0;i<bsz;i++) {
            if (beacons[i].on_main_road) continue;
            path.p.x=beacons[i].entrance.p.x;
            path.p.y=beacons[i].entrance.p.y;
            path.d=beacons[i].entrance.d;
            psz=beacons[i].path_out.size();
            beacons[i].path_in.resize(psz); //resize path_in

            for (int j=0;j<psz-1;j++) {
                beacons[i].path_in[j].p.x=beacons[i].path_out[psz-j-2].p.x;
                beacons[i].path_in[j].p.y=beacons[i].path_out[psz-j-2].p.y;
                beacons[i].path_in[j].d=(beacons[i].path_out[psz-j-1].d+2)%4;
            }
            beacons[i].path_in[psz-1].p.x=beacons[i].p.x;
            beacons[i].path_in[psz-1].p.y=beacons[i].p.y;
            beacons[i].path_in[psz-1].d=(beacons[i].path_out[0].d+2)%4;
        }
    }

    int recur_map_steps(Pos pt,int di,int depth,Pos pe,int de) {
        depth++;
        map_info[pt.x][pt.y].steps=depth;
        map_info[pt.x][pt.y].p=pe;
        map_info[pt.x][pt.y].d=de;

        Pos pb;

        vector<int> vd;
        Pos nextpos;
        int dt; // turn around direction
        int vsz,result;
        dt=(di+2)%4; // opposite direction, which is the coming direction
        for (int i=0;i<4;i++) { // to find out how many valid directions to go
            if (i==dt) continue; // do not search this
            if (hit_wall(pt,i)) continue; // not a valid way to go
            vd.push_back(i); // store possible directions of search
        }
        vsz=vd.size();
        if (vsz==0) { // reached dead end
            return 0;
        }

        for (int j=0;j<vd.size();j++) { // search further
            nextpos.x=pt.x+dir[vd[j]][0];
            nextpos.y=pt.y+dir[vd[j]][1];
            recur_map_steps(nextpos,vd[j],depth,pe,de);
        }
        return 0;
    }

    void init_map_info() {
        int vsz,pause,test,ways;
        Pos pt0,pt;
        vector<int> vd;
        int bn,result;
        Pos pb[5];

        for (int i=0;i<60;i++) {
            for (int j=0;j<40;j++) {
                vd.resize(0);
                if (map_info[i][j].type<0) continue;

                pt0.x=i;
                pt0.y=j;
                for (int di=0;di<4;di++) {
                    if (hit_wall(pt0,di)) continue;
                    vd.push_back(di);
                }
                vsz=vd.size();
                ways=0;
                for (int k=0;k<vsz;k++) {
                    pt.x=i+dir[vd[k]][0];
                    pt.y=j+dir[vd[k]][1];
                    if (map_info[pt.x][pt.y].type>=0) {
                        ways++;
                        continue;
                    }
                    // search alleyes only
                    map_info[i][j].type=1;
                    result=recur_map_steps(pt,vd[k],0,Pos(i,j),vd[k]);
                }
                map_info[i][j].ways=ways;
            }
        }
    }

    void ghosts_move() {
        int gsz;
        gsz=ghosts.size();
        for (int i=0;i<gsz;i++) {
            ghosts[i].track(player.pos());
        }
    }

    void new_game() {  // button click
        if (!map_ready) return;
        for (int i=0;i<5;i++) beacons[i].visited=false;
        continue_game();
    }

    void continue_game() { // button click
        if (!map_ready) return;
        deploy_runners();
    }

    void init_beacons() {
        Pos pt;
        Beacon bc;
        int xy[5][2]={0,0,48,0,48,22,0,22,25,17};
        beacons.resize(0);
        for (int i=0;i<5;i++) {
            pt.x=xy[i][0]+rand()%12;
            pt.y=xy[i][1]+rand()%8;
            bc.p=pt;
            if (map_info[pt.x][pt.y].type<0) {
                bc.on_main_road=false;
                bc.entrance.p=map_info[pt.x][pt.y].p;
                bc.entrance.d=map_info[pt.x][pt.y].d;
            } else {
                bc.on_main_road=true;
                bc.entrance.p=bc.p;
            }
            bc.visited=false;
            beacons.push_back(bc);
        }
        init_beacon_paths();
    }

    void clear_runners() {
        queue<Path> q;
        player.planned_path=q;
        player.walls=&walls;
        player.deployment=&deployment;
        player.map=&map;
        player.map_info=&map_info;
        player.protagonist();
        player.ghosts=&ghosts;
        player.beacons=&beacons;
        player.map_ready=&map_ready;

        bot.walls=&walls;
        bot.deployment=&deployment;
        bot.map=&map;
        bot.map_info=&map_info;
        bot.protagonist();
        bot.ghosts=&ghosts;
        bot.beacons=&beacons;

    }
    void mousedown(Pos pt) { // button click
        player.p=pt;
        player.nearest_entrance();
        player.planned_path=queue<Path>();
    }

    void clear_map() { //button click
        map_ready=false;
        memset (grid,0,sizeof(int)*(X+1)*(Y+1));
        memset (walls,0,sizeof(int)*X*Y*4);
        memset (map,0,sizeof(int)*X*Y);
        memset (map_depth,0,sizeof(int)*X*Y);
        memset (map_info,0,sizeof(Info)*X*Y);

        for (int x=0;x<=X;x++) {
            grid[x][0]=1;
            grid[x][Y]=1;
        }
        for (int y=0;y<=Y;y++) {
            grid[0][y]=1;
            grid[X][y]=1;
        }
    }

     void reset() {
        maze_ready=false;
        map_ready=false;
        clear_map();
        blue_print();
        clear_runners();
    }
};

