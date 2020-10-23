#include "plugin.hpp"


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

	Upasd() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BPM_SHIFT_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(25.4, 48.645)), module, Upasd::BPM_SHIFT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 119.415)), module, Upasd::IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 119.415)), module, Upasd::OUT_OUTPUT));

		// mm2px(Vec(40.64, 30.48))
		addChild(createWidget<Widget>(mm2px(Vec(5.08, 6.58))));
	}
};


Model* modelUpasd = createModel<Upasd, UpasdWidget>("upasd");