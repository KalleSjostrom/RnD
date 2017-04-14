using UnityEngine;
using System.Collections;
using System;

// http://en.wikipedia.org/wiki/Recurrent_neural_network
// Apply to training set, where the mean square error is the fitness function.
// Save load nn and rnn.
// Visualize them
// Different Sigmoidal functions, somtimes none? How to change?
// Different representation? The weights are stored in one long array.
// No reason to have a Neuron class.
// Is there any other way of storing the weights? Int array?! Some kind of tree-structure? (Why?)


public class Neuron {
	float[] weights;

	public Neuron(int size) {
		weights = new float[size+1];
		for (int i = 0; i < size; i++) {
			weights[i] = UnityEngine.Random.value;
		}
		float t = 0.5f;
		weights[size] = t;
	}
	public Neuron(float[] weights) {
		this.weights = weights;
	}

	public float Output(float[] input) {
		float activation = 0;
		for (int i = 0; i < input.Length; i++) {
			activation += input[i] * weights[i];
		}

		return Sigmoidal(activation);
	}

	float Sigmoidal(float activation) {
		// 1 / (1 + e^(-a/p)) where p = 1, a is the activation.
		float p = 1;
		return 1 / (1 + Mathf.Exp(-activation/p));
	}
}

public class NeuronLayer {
	private Neuron[] neurons;
	private float[] outputs;

	public NeuronLayer(int nrInputs, int nrOutputs, float[] weights) {
		neurons = new Neuron[nrOutputs];
		outputs = new float[nrOutputs];

		int counter = 0;
		for (int i = 0; i < neurons.Length; i++) {
			int neuronSize = nrInputs + 1;
			float[] dest = new float[neuronSize];
			Array.Copy(weights, counter, dest, 0, neuronSize);
			counter += neuronSize;
			neurons[i] = new Neuron(dest);
		}
	}

	public float[] Evaluate(float[] inputs) {
		for (int i = 0; i < neurons.Length; i++) {
			outputs[i] = neurons[i].Output(inputs);
		}
		return outputs;
	}
}

public class NeuralNet {
	private NeuronLayer[] layers;

	public NeuralNet(int[] layerSizes, float[] initialWeights) {
		// TODO: Clear distinction between the layers.
		DebugAux.Assert(layerSizes.Length >= 2, "Must have at least 2 layers");
		layers = new NeuronLayer[layerSizes.Length-1];
		int counter = 0;

		for (int i = 0; i < layers.Length; i++) {
			int size = (layerSizes[i]+1) * layerSizes[i+1];
			float[] dest = new float[size];
			Array.Copy(initialWeights, counter, dest, 0, size);
			counter += size;
			layers[i] = new NeuronLayer(layerSizes[i], layerSizes[i+1], dest);
		}
	}

	public float[] Evaluate(float[] inputs) {
		for (int i = 0; i < layers.Length; i++) {
			inputs = layers[i].Evaluate(inputs);
		}
		return inputs;
	}
}
