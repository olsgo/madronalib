
// MadronaLib: a C++ framework for DSP applications.
// Copyright (c) 2013 Madrona Labs LLC. http://www.madronalabs.com
// Distributed under the MIT license: http://madrona-labs.mit-license.org/

#ifndef __ML_SIGNALVIEWER_H__
#define __ML_SIGNALVIEWER_H__

#include "MLWidget.h"

#include "MLProcContainer.h"
#include "MLSymbol.h"
#include "MLPath.h"

#include "MLSignal.h"
#include "MLDSPEngine.h"

class MLSignalView
{
friend class MLSignalReporter;
public:
	static const char* kViewProcName;

	MLSignalView(MLWidget* w, MLSymbol attr, int size = kMLSignalViewBufferSize);
	~MLSignalView();
	void setupSignalView (MLDSPEngine* pEng, const MLSymbol sigName, unsigned voices);
	void sendSignalToWidget(const MLSignal& signal, int samples);
	
	const MLRect& getRepaintRect() const;	
	MLSymbol getSignalName() { return mSignalName; }	

private:

	int mNumSignals;
	bool mViewingSignal;
	MLDSPEngine* mpEngine;
	
	MLSymbol mSignalName;
	MLWidget* mpWidget;
	MLSymbol mAttr;
	int mSize;
};



#endif