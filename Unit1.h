//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <jpeg.hpp>




//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
    TButton *Startstop;
    TButton *Reset;
    TTimer *Timer1;
    TButton *Step;
    TButton *Newgame;
    TCheckBox *Freeway;
    TImage *Image1;
    TComboBox *ComboBox1;
    TLabel *Label1;
    TLabel *Label2;
    TCheckBox *Radar;
    TCheckBox *Guide;
    TButton *Aimove;
    TTimer *Timer3;
    TButton *Faster;
    TButton *Slower;
    TButton *Continue;
    TButton *Auto;
    TTimer *Timer2;
    TImage *Image2;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label6;
    TEdit *Edit1;
    void __fastcall StartstopClick(TObject *Sender);
    void __fastcall ResetClick(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall StepClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FreewayClick(TObject *Sender);
    void __fastcall Image1MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall NewgameClick(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall RadarClick(TObject *Sender);
    void __fastcall GuideClick(TObject *Sender);
    void __fastcall AimoveClick(TObject *Sender);
    void __fastcall Timer3Timer(TObject *Sender);
    void __fastcall ComboBox1Change(TObject *Sender);
    void __fastcall SlowerClick(TObject *Sender);
    void __fastcall FasterClick(TObject *Sender);
    void __fastcall ContinueClick(TObject *Sender);
    void __fastcall AutoClick(TObject *Sender);
    void __fastcall Timer2Timer(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);

};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
