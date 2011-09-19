//
//  Envelopes.h
//  
//
//  Created by Terry Appleby on 11-09-13.
//  Copyright 2011 leet powered. All rights reserved.
//

#ifndef __ENVELOPES__
#define __ENVELOPES__

enum EnvelopeState {
    ENVS_IDLE = 0,
    ENVS_ATTACK,
    ENVS_DECAY,
    ENVS_SUSTAIN,
    ENVS_RELEASE
};

static const char* envID = "IADSR";

static const double ONE_SECOND = 1000.0f;

class EnvADSR {
private:
    double fSampleRate;
    double fAttack;
    double fDecay;
    double fSustain;
    double fRelease;
    double fCurrentValue;
    
    
    EnvelopeState state;
    bool          bGate;
    
    
    
public:
    EnvADSR(double sampleRate, double attack=0.0f, double decay=50.0f, double sustain=1.0f, double release=0.0f) : 
        fSampleRate(sampleRate),
        fAttack(attack),
        fDecay(decay),
        fSustain(sustain),
        fRelease(release),
        fCurrentValue(0.0f),
        state(ENVS_IDLE),
        bGate(false)
        
    {
        
    }
    
    void setGate(bool gateValue)
    {
        if(gateValue != bGate) //Only if gate has changed
        {
            bGate = gateValue;
            state = bGate ? ENVS_ATTACK : ENVS_RELEASE;
        }
    }
    
    void setSampleRate(double sampleRate) { fSampleRate = sampleRate; }
    void setAttack(double attack) { fAttack = attack; }
    void setDecay(double decay) { fDecay = decay; }
    void setSustain(double sustain) { fSustain = sustain; }
    void setRelease(double release) { fRelease = release; }
    
    void setADSR(double attack, double decay, double sustain, double release)
    {
        fAttack = attack;
        fDecay = decay;
        fSustain = sustain;
        fRelease = release;
    }
    
    double getSampleRate() { return fSampleRate; }
    double getAttack() { return fAttack; }
    double getDecay() { return fDecay; }
    double getSustain() { return fSustain; }
    double getRelease() { return fRelease; }
    
    double getCurrentValue() { return fCurrentValue; }
    
    EnvelopeState getState() { return state; }
    
    double update()
    {
        switch (state) 
        {
            case ENVS_ATTACK:
            {
                fCurrentValue += ONE_SECOND / fSampleRate / fAttack;
                
                if(fCurrentValue >= 1.0)
                {
                    fCurrentValue = 1.0f;
                    state = ENVS_DECAY;
                }
                break;
            }
            
            case ENVS_DECAY:
            {
                fCurrentValue -= ONE_SECOND / fSampleRate / fDecay * fSustain;
                
                if(fCurrentValue <= fSustain)
                {
                    fCurrentValue = fSustain;
                    state = ENVS_SUSTAIN;
                }
                
                break;
            }
            
            case ENVS_SUSTAIN:
            {
                fCurrentValue = fSustain;
                break;
            }
                
            case ENVS_RELEASE:
            {
                fCurrentValue -= ONE_SECOND / fSampleRate / fRelease;
                if(fCurrentValue <= 0.0f)
                {
                    fCurrentValue = 0.0f;
                    state = ENVS_IDLE;
                }
                break;
            }
                
            default:
                break;
        }
        
        return fCurrentValue;
    }
};


#endif