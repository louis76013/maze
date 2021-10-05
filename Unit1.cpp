//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "maze.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
Maze maze;

void update_map();

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
    Caption="louis76013@gmail.com";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::StartstopClick(TObject *Sender)
{
    DefocusControl(Startstop,true);

    if (Timer1->Enabled) {
        Timer1->Enabled=False;
        return;
    }

    if (!Timer1->Enabled) {
        Timer1->Enabled=True;
        return;
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ResetClick(TObject *Sender)
{
    int X,Y,x2,y2;
    x2=Image1->Width;
    y2=Image1->Height;
    Image1->Canvas->Brush->Color = clBtnFace;
    Image1->Canvas->FillRect(Rect(0,0,x2,y2));

    maze.reset();

    X=maze.X;
    Y=maze.Y;
    Image1->Canvas->Pen->Color = clBlack;
    Image1->Canvas->MoveTo(X*15+20,Y*15+20);
    Image1->Canvas->LineTo(0*15+20,Y*15+20);
    Image1->Canvas->LineTo(0*15+20,0*15+20);
    Image1->Canvas->LineTo(X*15+20,0*15+20);
    Image1->Canvas->LineTo(X*15+20,Y*15+20);
    DefocusControl(Reset,true);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender) // build walls
{
    Pos pt;
    Line line;
    Pos p1,p2;
    int csz,qsz;

    if (maze.is_maze_ready()) {
        Timer1->Enabled =False;
        return;
    }
    maze.grow_blocks();
    Image1->Canvas->Pen->Color=clBlack;
    csz=maze.blocks.size();

    for (int i=0;i<csz;i++) {
        qsz=maze.blocks[i].qln.size();
        line=maze.blocks[i].qln.back();
        p1=line.p1;
        p2=line.p2;
        Image1->Canvas->MoveTo(p1.x*15+20,p1.y*15+20);
        Image1->Canvas->LineTo(p2.x*15+20,p2.y*15+20);

    }
    return;
}
//---------------------------------------------------------------------------
void maze_progression() {
    Pos pt;
    Line line;
    Pos p1,p2;
    int csz,qsz;

    if (maze.is_map_ready()) {
        Form1->Timer1->Enabled =False;
        return;
    }

    maze.grow_blocks();

    csz=maze.blocks.size();

    for (int i=0;i<csz;i++) {
        qsz=maze.blocks[i].qln.size();
        line=maze.blocks[i].qln.back();
        p1=line.p1;
        p2=line.p2;
        Form1->Image1->Canvas->MoveTo(p1.x*15+20,p1.y*15+20);
        Form1->Image1->Canvas->LineTo(p2.x*15+20,p2.y*15+20);
    }
}
//------------------------------------------

void __fastcall TForm1::StepClick(TObject *Sender)
{
    maze_progression();
    DefocusControl(Step,true);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormShow(TObject *Sender)
{
    int X,Y;
    X=maze.X;
    Y=maze.Y;
    Image1->Height=Y*15+40;
    Image1->Width=X*15+40;

    ResetClick(Reset);

    if (ComboBox1->ItemIndex>=0) {
         maze.N=ComboBox1->ItemIndex+1;
    } else maze.N=5;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::FreewayClick(TObject *Sender)
{
    maze.set_freeway(Freeway->Checked);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Image1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (X<20 || X>maze.X*15+20) return;
    if (Y<20 || Y>maze.Y*15+20) return;

    Pos p;
    p=Pos((X-20)/15,(Y-20)/15);
    maze.mousedown(p);
    update_map();
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TForm1::NewgameClick(TObject *Sender)
{
    if (!maze.map_ready) {
        DefocusControl(Newgame,true);
        return;
    }
    Timer3->Enabled=false;
    maze.new_game();
    update_map();
    DefocusControl(Newgame,true);
    return;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    int di,gsz,result;
    if (Key<37 || Key>40) {
        if (Key !=32) return;
    }
    if (maze.is_game_over()) {
        return;
    }

    bool player_moved=false;

    if (Key!=32) {
        di=int(Key)-37;
        if (maze.player.move(di)) {
            player_moved=true;
            maze.update_beacons();
            if (maze.is_game_over()) {
                update_map();
                ShowMessage("Game Over");
                return;
            }
            result=maze.player.recur_search(maze.player.pos(),maze.player.d,20);
        }
    }

    if (Key==32 || player_moved) {
        gsz=maze.ghosts.size();
        for (int i=0;i<gsz;i++) {
            maze.ghosts[i].track(maze.player.pos());
        }
    }
    update_map();

}
//---------------------------------------------------------------------------


void __fastcall TForm1::RadarClick(TObject *Sender)
{
    maze.set_radar(Radar->Checked);
    update_map();
    DefocusControl(Radar,true);
}
//---------------------------------------------------------------------------


void update_map() {
    int x1,y1;
    int gsz,bsz;
    maze.update_map();

    for (int i=0;i<60;i++) {
        for (int j=0;j<40;j++) {
            x1=i*15+23;
            y1=j*15+23;
            Form1->Image1->Canvas->Brush->Color =clBtnFace;
            Form1->Image1->Canvas->Pen->Color =clBtnFace;
            Form1->Image1->Canvas->FillRect(Rect(x1,y1,x1+10,y1+10));
            if (maze.map[i][j]%10) { // hint
                Form1->Image1->Canvas->Brush->Color =clMoneyGreen;
                Form1->Image1->Canvas->Pen->Color =clMoneyGreen;
                Form1->Image1->Canvas->FillRect(Rect(x1,y1,x1+10,y1+10));

            }
            if (maze.map[i][j]/1000) { // ghost
                Form1->Image1->Canvas->Brush->Color =clRed;
                Form1->Image1->Canvas->Pen->Color =clRed;
                Form1->Image1->Canvas->Ellipse(x1,y1,x1+10,y1+10);

            } else if ((maze.map[i][j]/100)%10) { // player
                Form1->Image1->Canvas->Brush->Color =clBlue;
                Form1->Image1->Canvas->Pen->Color =clBlue;
                Form1->Image1->Canvas->Ellipse(x1,y1,x1+10,y1+10);


            } else if ((maze.map[i][j]/10)%10) { // beacon
                Form1->Image1->Canvas->Brush->Color =clGreen;
                Form1->Image1->Canvas->Pen->Color =clGreen;
                Form1->Image1->Canvas->Ellipse(x1,y1,x1+10,y1+10);

            }
        }
    }
}

void __fastcall TForm1::GuideClick(TObject *Sender)
{
    maze.set_guide(Guide->Checked);
    update_map();
    DefocusControl(Guide,true);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::AimoveClick(TObject *Sender)
{
    if (!maze.is_map_ready()) {
        DefocusControl(Aimove,true);
        return;
    }

    if (Timer3->Enabled) {
        Timer3->Enabled=false;

    } else Timer3->Enabled=true;

    DefocusControl(Aimove,true);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer3Timer(TObject *Sender)
{
    maze.player.ai_move();
    maze.update_beacons();
    maze.ghosts_move();
    if (maze.is_game_over()) {
        Timer3->Enabled=false;
    }
    update_map();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ComboBox1Change(TObject *Sender)
{
    int i=ComboBox1->ItemIndex;
    maze.N=StrToInt(ComboBox1->Items->Strings[i]);
    DefocusControl(ComboBox1,true);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::SlowerClick(TObject *Sender)
{
    Timer3->Interval+=10;
    DefocusControl(Slower,true);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FasterClick(TObject *Sender)
{
    if (Timer3->Interval>=20) Timer3->Interval-=10;
    DefocusControl(Faster,true);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::ContinueClick(TObject *Sender)
{
    maze.continue_game();
    update_map();
    DefocusControl(Continue,true);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::AutoClick(TObject *Sender)
{
    ResetClick(Reset);
    Timer1->Enabled=true;
    Timer2->Enabled=true;
    DefocusControl(Auto,true);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer2Timer(TObject *Sender)
{
    if (maze.is_map_ready()) {
        Timer2->Enabled=false;
        AimoveClick(Aimove);
    }
}
//---------------------------------------------------------------------------

