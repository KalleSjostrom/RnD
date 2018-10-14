#include "engine/utils/math/random.h"

// http://en.wikipedia.org/wiki/Recurrent_neural_network
// Apply to training set, where the mean square error is the fitness function.
// Save load nn and rnn.
// Visualize them
// Different Sigmoidal functions, somtimes none? How to change?
// Different representation? The weights are stored in one long array.
// No reason to have a Neuron class.
// Is there any other way of storing the weights? Int array?! Some kind of tree-structure? (Why?)

struct FloatArray {
	float *values;
	i32 count;
	f32 operator[](int index) {
		ASSERT(index < count, "Out of bounds!");
		return values[index];
	}
};

struct NeuronLayer {
	float **neurons;
	FloatArray output;
	i32 neuron_count;
};

struct NeuralNet {
	NeuronLayer *layers;
	i32 layer_count;
};

struct NeuronInputOutput {
	float *input;
	i32 count;
};

void init_neuron_layer(ArenaAllocator &arena, NeuronLayer &layer, i32 input_count, i32 output_count, FloatArray weights) {
	layer.neurons = PUSH_STRUCTS(arena, output_count, float*);
	layer.output.values = PUSH_STRUCTS(arena, output_count, float);
	layer.output.count = output_count;

	i32 counter = 0;
	for (i32 i = 0; i < output_count; i++) {
		i32 neuron_count = input_count + 1;

		layer.neurons[i] = weights.values + neuron_count;

		counter += neuron_count;
	}
}

NeuralNet make_neural_net(ArenaAllocator &arena, i32 *layer_sizes, i32 layer_size_count, float *weights, i32 weight_count) {
	ASSERT(layer_size_count >= 2, "Must have at least 2 layers");

	i32 layer_count = layer_size_count-1;

	NeuralNet net = {};
	net.layers = PUSH_STRUCTS(arena, layer_count, NeuronLayer);
	net.layer_count = layer_count;

	i32 counter = 0;
	for (i32 i = 0; i < layer_count; i++) {
		i32 size = (layer_sizes[i]+1) * layer_sizes[i+1];

		FloatArray w = { weights + counter, size };
		init_neuron_layer(arena, net.layers[i], layer_sizes[i], layer_sizes[i+1], w);

		counter += size;
	}
	return net;
}

FloatArray evaluate_neural_net(NeuralNet &net, FloatArray inputs) {
	for (i32 i = 0; i < net.layer_count; i++) {
		NeuronLayer &layer = net.layers[i];
		FloatArray &layer_input = i == 0 ? inputs : net.layers[i-1].output;
		for (i32 j = 0; j < layer.neuron_count; j++) {
			float *neuron_weights = layer.neurons[j];
			float activation = 0;
			// Variable length dot-product, make it wide?
			for (i32 k = 0; k < layer_input.count; k++) {
				activation += layer_input[j] * neuron_weights[k];
			}

			layer.output.values[j] = sigmoidal(activation);
		}
	}
	return net.layers[net.layer_count-1].output;
}
