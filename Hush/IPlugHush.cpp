#include "IPlugHush.h"
#include "IPlug/IPlug_include_in_plug_src.h"
#include "IPlug/IPopupControl.h"

#include "resource.h"
#include <math.h>

const int kNumPrograms = 1;

enum EParams 
{
	kMidiKey = 0,
    kGateType,
    kAttack,
    kDecay,
    kSustain,
    kRelease,
	kNumParams
};

enum EHushCommand 
{
    EHC_MidiAny = 0,
    EHC_MidiLearn
};

enum EGateType {
	EGT_Up = 0,
	EGT_Toggle,
	EGT_Down,
	EGT_Max
};

enum EChannelSwitch 
{
	kDefault = 0,
	kReversed,
	kAllLeft,
	kAllRight,
	kOff,
	kNumChannelSwitchEnums
};

enum ELayout
{
	kW = 300,
	kH = 150,
};

static const char* g_KeyNames[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

void GetKeyName(char* pBuff, int keyIndex)
{
    if(keyIndex >= 0)
    {
        int modVal = keyIndex % 12;
        int idx = (keyIndex / 12) - 2;
        
        sprintf(pBuff, "%s %d", g_KeyNames[modVal], idx);
    }
    else if(keyIndex == -1)
    {
        strcpy(pBuff, "Any");
    }
    else
    {
        strcpy(pBuff, "");
    }
}

class ICommandControl : public IControl
{
public:
    ICommandControl(IPlugBase* pPlug, IRECT* pRect, int paramIdx, int nCommand, IChannelBlend::EBlendMethod blendMethod = IChannelBlend::kBlendNone) 
	:	IControl(pPlug, pRect, paramIdx, blendMethod), m_nCommand(nCommand) {}
    
	ICommandControl(IPlugBase* pPlug, IRECT* pRect, int nCommand,  IChannelBlend::EBlendMethod blendMethod = IChannelBlend::kBlendNone)
	:	IControl(pPlug, pRect, -1, blendMethod), m_nCommand(nCommand) {}
    
	virtual ~ICommandControl() {}

    void OnMouseDblClick(int x, int y, IMouseMod* pMod)
    {
        static_cast<PlugHush*>(mPlug)->OnCustomCommand(m_nCommand, 0);
    }
    
    virtual bool Draw(IGraphics* pGraphics)
    {
        return true;
    }
protected:
    int m_nCommand;
};

//Some custom controls
class ICommandBitmapControl : public IBitmapControl
{
public:
    
	ICommandBitmapControl(IPlugBase* pPlug, int x, int y, int paramIdx, IBitmap* pBitmap, int nCommand, IChannelBlend::EBlendMethod blendMethod = IChannelBlend::kBlendNone) 
	:	IBitmapControl(pPlug, x, y, paramIdx, pBitmap, blendMethod), m_nCommand(nCommand) {}
    
	ICommandBitmapControl(IPlugBase* pPlug, int x, int y, IBitmap* pBitmap, int nCommand,  IChannelBlend::EBlendMethod blendMethod = IChannelBlend::kBlendNone)
	:	IBitmapControl(pPlug, x, y, -1, pBitmap, blendMethod), m_nCommand(nCommand) {}
    
	virtual ~ICommandBitmapControl() {}
    
    void OnMouseDblClick(int x, int y, IMouseMod* pMod)
    {
        static_cast<PlugHush*>(mPlug)->OnCustomCommand(m_nCommand, 0);
    }
protected:
    int m_nCommand;
};




PlugHush::PlugHush(IPlugInstanceInfo instanceInfo)
:	IPLUG_CTOR(kNumParams, 1, instanceInfo), prevL(0.0), prevR(0.0), m_nGainPct(1.0), m_nNote(-1), m_bMidiLearnEnabled(false), m_ADSR( GetSampleRate() )
{
  TRACE;

	// Define parameter ranges, display units, labels.

    /*const char* name, double defaultVal, double minVal, double maxVal, double step, const char* label*/
    

	//GetParam(kMidiKey)->InitInt("Key", -1, -1, 128, "");
    GetParam(kMidiKey)->InitEnum("Key", 0, 121);
    
    for(int i=0; i < 122; i++)
    {
        char buff[16];
        GetKeyName(buff, i-1);
        GetParam(kMidiKey)->SetDisplayText(i, buff);
    }
    
    m_ADSR.setADSR(30.0, 0.0, 1.0, 30.0);
    
    GetParam(kGateType)->InitEnum("Type", EGT_Toggle, EGT_Max);
	GetParam(kGateType)->SetDisplayText(EGT_Up, "up");
	GetParam(kGateType)->SetDisplayText(EGT_Toggle, "toggle");
	GetParam(kGateType)->SetDisplayText(EGT_Down, "down");
    
    GetParam(kAttack)->InitDouble("Attack", m_ADSR.getAttack(), 0.0f, 1000.0f, 0.1f, "ms");
    GetParam(kDecay)->InitDouble("Decay", m_ADSR.getDecay(), 0.0f, 1000.0f, 0.1f, "ms");
    GetParam(kSustain)->InitDouble("Sustain", m_ADSR.getSustain(), 0.0f, 1.0f, 0.01f, "%");
    GetParam(kRelease)->InitDouble("Release", m_ADSR.getRelease(), 0.0f, 1000.0f, 0.1f, "ms");

    
    MakeDefaultPreset("Default");

	// Instantiate a graphics engine.

    IBitmap bitmap;
    
	IGraphics* pGraphics = MakeGraphics(this, kW, kH);
	
    //Background
    pGraphics->AttachBackground(BG_ID, BG_FN);

    //Midi area
    
    bitmap = pGraphics->LoadIBitmap(IMG_MIDIBG_ID, IMG_MIDIBG_FN);
    pGraphics->AttachControl( new IBitmapControl(this, 8, 0, -1, &bitmap) );
    //pGraphics->AttachControl( new ICommandBitmapControl(this, 8,0, -1, &bitmap, EHC_MidiLearn) );
    
    bitmap = pGraphics->LoadIBitmap(IMG_MIDI_ID, IMG_MIDI_FN);
    pGraphics->AttachControl( new IBitmapControl(this, 12,6, -1, &bitmap) );

    IRECT fontLocation(34, 5, 34+40, 5+14);
    IText lFont(12, &COLOR_WHITE, "Arial", IText::kStyleBold, IText::kAlignNear, 0, IText::kQualityClearType);
    m_nMidiTextIdx = pGraphics->AttachControl( new ITextControl(this, &fontLocation, &lFont, "") );
    
    IRECT midiAreaHotSpot(10, 0, 10+58, 0+21);
    pGraphics->AttachControl( new ICommandControl(this, &midiAreaHotSpot, -1, EHC_MidiLearn) );
    
    //LED control
    bitmap = pGraphics->LoadIBitmap(IMG_LED_ID, IMG_LED_FN, 20);
	m_nLEDIdx = pGraphics->AttachControl(new ISwitchControl(this, 6, 24, -1, &bitmap));
    
    //Gate type knob
    bitmap = pGraphics->LoadIBitmap(IMG_KNOB1_ID, IMG_KNOB1_FN, 3);
    pGraphics->AttachControl(new IKnobMultiControl(this, 36, 28, kGateType, &bitmap, kVertical, 1));
	//pGraphics->AttachControl(new ISwitchControl(this, 36, 28, kGateType, &bitmap));
    
    //ADSR knobs
    bitmap = pGraphics->LoadIBitmap(IMG_KNOB2_ID, IMG_KNOB2_FN, 34);
    
    
	pGraphics->AttachControl(new IKnobMultiControl(this, 166, 14, kAttack, &bitmap, kVertical, DEFAULT_GEARING*0.5));
    pGraphics->AttachControl(new IKnobMultiControl(this, 237, 14, kDecay, &bitmap, kVertical, DEFAULT_GEARING*0.5));
    pGraphics->AttachControl(new IKnobMultiControl(this, 156, 84, kSustain, &bitmap, kVertical, DEFAULT_GEARING*0.5));
    pGraphics->AttachControl(new IKnobMultiControl(this, 226, 84, kRelease, &bitmap, kVertical, DEFAULT_GEARING*0.5));
    
    //Help area
    bitmap = pGraphics->LoadIBitmap(IMG_HELPICON_ID, IMG_HELPICON_FN);
    
    IBitmapControl* pHelpIconCtrl = new IBitmapControl(this, 276, 126, -1, &bitmap);
    pGraphics->AttachControl( pHelpIconCtrl );
    
    IRECT* pTargetRect = pHelpIconCtrl->GetRECT();
    
    bitmap = pGraphics->LoadIBitmap(IMG_HELPDESC_ID, IMG_HELPDESC_FN);
    pGraphics->AttachControl( new IBitmapOverlayControl(this, 0,0, -1, &bitmap, pTargetRect) );
    //IBitmapOverlayControl(IPlugBase* pPlug, int x, int y, int paramIdx, IBitmap* pBitmap, IRECT* pTargetArea)
    
   
    //Debug text
    //IText lFont(12, &COLOR_RED);
    
    /*IText(int size = DEFAULT_TEXT_SIZE, const IColor* pColor = 0, char* font = 0,
     EStyle style = kStyleNormal, EAlign align = kAlignCenter, int orientation = 0, EQuality quality = kQualityDefault)*/
    

    //ITextControl* pTestCtrl = new ITextControl(this, &fontLocation, &lFont, "500");
    //m_nTextIdx = pGraphics->AttachControl(pTestCtrl);

	AttachGraphics(pGraphics);

	// No cleanup necessary, the graphics engine manages all of its resources and cleans up when closed.
}

void PlugHush::SetMidiAreaText(char* pText, const IColor* color)
{
    if( GetGUI() )
    {
        ITextControl* pTextCtrl = (ITextControl*)(GetGUI()->GetControl(m_nMidiTextIdx));
        pTextCtrl->SetTextFromPlug(pText);
        
        //pTextCtrl->GetIText()->mColor.A = color->A;// (, color->R, color->G, color->B);
        pTextCtrl->GetITTextRef().mColor.A = color->A;
        pTextCtrl->GetITTextRef().mColor.R = color->R;
        pTextCtrl->GetITTextRef().mColor.G = color->G;
        pTextCtrl->GetITTextRef().mColor.B = color->B;
    }

}

void PlugHush::SetMidiAreaKey(int index, const IColor* color)
{
    char buff[16];
    GetKeyName(buff, index);
    
    SetMidiAreaText(buff, color);
}

void PlugHush::Reset()
{
    int listenKey = GetParam(kMidiKey)->Int() - 1;
    SetMidiAreaKey( listenKey , &COLOR_WHITE);
    
    m_ADSR.setSampleRate( GetSampleRate() );
    
    double fAttack = GetParam(kAttack)->Value();
    double fDecay = GetParam(kDecay)->Value();
    double fSustain = GetParam(kSustain)->Value();
    double fRelease = GetParam(kRelease)->Value();
    
    m_ADSR.setADSR(fAttack, fDecay, fSustain, fRelease);
}

void PlugHush::OnParamChange(int paramIdx)
{
    if(paramIdx == kMidiKey || paramIdx == kGateType)
    {
        m_nNote = -1;
        
        int listenKey = GetParam(kMidiKey)->Int() - 1;
        SetMidiAreaKey( listenKey , &COLOR_WHITE);
    }
    else if(paramIdx >= kAttack && paramIdx <= kRelease)
    {
        double fAttack = GetParam(kAttack)->Value();
        double fDecay = GetParam(kDecay)->Value();
        double fSustain = GetParam(kSustain)->Value();
        double fRelease = GetParam(kRelease)->Value();
        
        m_ADSR.setADSR(fAttack, fDecay, fSustain, fRelease);
    }
}

void PlugHush::OnCustomCommand(int commandID, int nAction)
{
    switch (commandID) {
        case EHC_MidiLearn:
        {
            if(m_bMidiLearnEnabled) //Midi learn enabled? set back to any key
            {
                GetParam(kMidiKey)->Set( 0 );

                InformHostOfParamChange(kMidiKey, GetParam(kMidiKey)->GetNormalized());
                
                SetMidiAreaKey(-1, &COLOR_WHITE);
                
                m_bMidiLearnEnabled = false;
            }
            else
            {
                SetMidiAreaText("----", &COLOR_RED);
                m_bMidiLearnEnabled = true;
            }
            
            break;
        }   
        default:
            break;
    }
}

void PlugHush::ProcessMidiMsg(IMidiMsg* pMsg)
{
    int status = pMsg->StatusMsg();
    
    int listenKey = GetParam(kMidiKey)->Int() - 1;
    
    
    switch (status)
    {
        case IMidiMsg::kNoteOn:
        case IMidiMsg::kNoteOff:
            
            if(m_bMidiLearnEnabled)
            {
                GetParam(kMidiKey)->Set( double( pMsg->NoteNumber()+1 ) );
                m_bMidiLearnEnabled = false;
                
                InformHostOfParamChange(kMidiKey, GetParam(kMidiKey)->GetNormalized());
                
                SetMidiAreaKey( pMsg->NoteNumber() , &COLOR_WHITE);
                
                return;
            }
            
            if(listenKey != -1 && pMsg->NoteNumber() != listenKey)
                return; //Dont add to queue if were not listening for you!
            
            break;
        // Discard all other MIDI messages.
		default:
			SendMidiMsg(pMsg);
			return;
    }
    
    
    m_oMidiQueue.Add(pMsg);
    
    /*if( GetGUI() )
    {
        ITextControl* pTestCtrl = (ITextControl*)(GetGUI()->GetControl(m_nTextIdx));
        
        char buff[128];
        
        sprintf(buff, "Note: %d | GainPct: %f", pMsg->NoteNumber(), m_nGainPct);
        
        pTestCtrl->SetTextFromPlug(buff);
    }*/
}

void PlugHush::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

    double* in1 = inputs[0];
    double* in2 = inputs[1];
    double* out1 = outputs[0];
    double* out2 = outputs[1];

    
    //double peakL = 0.0, peakR = 0.0;
    
    
    static int tLastMidiNote = -1;
    
    EGateType gateType = (EGateType)GetParam(kGateType)->Int();
    
    double peakMidiGate = 0.0f;

    for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2) 
    {
        while (!m_oMidiQueue.Empty())
		{
			IMidiMsg* pMsg = m_oMidiQueue.Peek();
			// Stop when we've reached the current sample frame (offset).
			if (pMsg->mOffset > s)
				break;
            
            // Handle the MIDI message.
			int status = pMsg->StatusMsg();
			switch (status)
			{
				case IMidiMsg::kNoteOn:
				case IMidiMsg::kNoteOff:
				{
					int velocity = pMsg->Velocity();
                    
                    tLastMidiNote = pMsg->NoteNumber();
                    

					if (status == IMidiMsg::kNoteOn && velocity)
					{
                        if(gateType == EGT_Toggle && m_nNote != -1) //Were in toggle and note is set, turn it off
                            m_nNote = -1;
                        else
                            m_nNote = pMsg->NoteNumber(); //All other cases we turn note on
                        
					}
					// Note Off
					else
					{
						if (pMsg->NoteNumber() == m_nNote && gateType != EGT_Toggle) //If were in toggle mode, dont reset
							m_nNote = -1;
					}
					break;
				}
			}
            
			// Delete the MIDI message we've just handled from the queue.
			m_oMidiQueue.Remove();
        }
        
        if(m_nNote != -1)
        {
            //m_nGainPct = 0;
            m_ADSR.setGate(true);
        }
        else
        {
            m_ADSR.setGate(false);
        }
        
        
        if(gateType == EGT_Down) 
            m_nGainPct = m_ADSR.update(); //No need to negate it
        else
            m_nGainPct = 1.0 - m_ADSR.update();

        *out1 = *in1 * m_nGainPct;
        *out2 = *in2 * m_nGainPct;


        //peakL = MAX(peakL, fabs(*out1));
        //peakR = MAX(peakR, fabs(*out2));
        
        //peakMidiGate = MAX(peakMidiGate, m_nGainPct);
    }

    //const double METER_ATTACK = 0.6, METER_DECAY = 0.1;
    //double xL = (peakL < prevL ? METER_DECAY : METER_ATTACK);
    //double xR = (peakR < prevR ? METER_DECAY : METER_ATTACK);

    //peakL = peakL * xL + prevL * (1.0 - xL);
    //peakR = peakR * xR + prevR * (1.0 - xR);

    //prevL = peakL;
	//prevR = peakR;
    
    // Update the offsets of any MIDI messages still in the queue.
	m_oMidiQueue.Flush(nFrames);
    
    if( GetGUI() )
    {        
        GetGUI()->SetControlFromPlug(m_nLEDIdx, m_nGainPct);
    }
}


