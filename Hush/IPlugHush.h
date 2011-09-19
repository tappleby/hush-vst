#ifndef __PLUGHUSH__
#define __PLUGHUSH__

// In the project settings, define either VST_API or AU_API.
#include "IPlug/IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"
#include "Envelopes.h"

class PlugHush : public IPlug
{
public:

	PlugHush(IPlugInstanceInfo instanceInfo);
	~PlugHush() {}	// Nothing to clean up.

	// Implement these if your audio or GUI logic requires doing something 
	// when params change or when audio processing stops/starts.
	void Reset();
	void OnParamChange(int paramIdx);

    void ProcessMidiMsg(IMidiMsg* pMsg);
	void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
    
    void OnCustomCommand(int commandID, int nAction);

    void SetMidiAreaText(char* pText, const IColor* color);
    void SetMidiAreaKey(int index, const IColor* color);
private:

    
    
    int m_nNote;
    
	int mMeterIdx_L, mMeterIdx_R;
	double prevL, prevR;
    
    double m_nGainPct;
    
    int m_nTextIdx;
    int m_nMidiTextIdx;
    
    int m_nLEDIdx;
    
    IMidiQueue m_oMidiQueue;
    
    bool m_bMidiLearnEnabled;
    
    EnvADSR m_ADSR;
    
};

////////////////////////////////////////

#endif
