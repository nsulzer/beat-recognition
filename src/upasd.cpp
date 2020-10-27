#include "plugin.hpp"
#include <math.hpp>


struct Upasd : Module {
	enum ParamIds {
		BPM_SHIFT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	#define WINDOWSIZE 1024
	#define HOPSIZE 128

	float hamm[WINDOWSIZE] = {0.f}; // Hamming window constants.

	Upasd() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BPM_SHIFT_PARAM, -5.f, 0.f, 5.f, "BPM Scale","V/div");
		for (int i=0; i < WINDOWSIZE; ++i) {
			hamm[i] = 0.54 - 0.46 * cos( 2*M_PI*i / (WINDOWSIZE-1)); // Pre-compute Hamming window constants, consider LUT?
		}
	}

	float vsum (float arr[]) {
		// calculate sum of arrray elements
		int arr_size = sizeof(arr)/sizeof(arr[0]);
		float summ = 0.f;
		for (int i = 0; i < arr_size; ++i) {
			summ += arr[i];
		}
		return summ;
	}

	int i_buf = HOPSIZE -1;
	float audioFrame[WINDOWSIZE] = {0.f};
	float bpmShift = 0.f;
	alignas(16) float windows[WINDOWSIZE] = {0.f};
	alignas(16) float fft_res[WINDOWSIZE*2] = {0.f};
	alignas(16) float diff[WINDOWSIZE], logmag[WINDOWSIZE] = {0.f};

	void process(const ProcessArgs& args) override {
		int i;
		audioFrame[i_buf] = inputs[IN_INPUT].getVoltageSum();
		windows[i_buf] = hamm[i_buf] * audioFrame[i_buf];
		i_buf--;
		if (i_buf < 0 ) { // 128 samples read
			// compute fft, etc.
			float flux = 0.f;
			dsp::RealFFT fft(WINDOWSIZE);
			fft.rfftUnordered(windows, fft_res);
			for(i = 0; i < WINDOWSIZE/2; ++i){
				if(fft_res[i] <= 0 ){
					logmag[i] = log10(1-fft_res[i]*1000/WINDOWSIZE) - logmag[i];
				}
				else{
					logmag[i] = log10(1+fft_res[i]*1000/WINDOWSIZE) - logmag[i];
				}

				if (logmag[i] > 0) {
					flux += logmag[i];
				}
			}


			// fft_res = fft(windows);
			// fftabs = abs(fft_res);
			// fftabs_reduced = fftabs(1:WINDOWSIZE/2+1) / WINDOWSIZE;
			// logmag = log(1+1000*fftabs_reduced);

			i_buf = HOPSIZE - 1; // reset i_buf
			for (i=WINDOWSIZE-HOPSIZE-1; i > 0; --i) { // move first 896 samples to end of frame, multiply with window
				audioFrame[i+HOPSIZE] = audioFrame[i];
				windows[i] = hamm[i] * audioFrame[i];
			}

			// float flux = vsum(logmag);
			outputs[OUT_OUTPUT].setVoltage(flux); // change this to actual output.
		}
		else { outputs[OUT_OUTPUT].setVoltage(0.f);
		}
	}
};


struct UpasdWidget : ModuleWidget {
	UpasdWidget(Upasd* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/upasd.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(25.4, 48.645)), module, Upasd::BPM_SHIFT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 119.415)), module, Upasd::IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 119.415)), module, Upasd::OUT_OUTPUT));

		// mm2px(Vec(40.64, 30.48))
		addChild(createWidget<Widget>(mm2px(Vec(5.08, 6.58))));
	}
};


Model* modelUpasd = createModel<Upasd, UpasdWidget>("upasd");