using UnityEngine;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System;

struct Individual {
	public uint genome;
	public int fitness;
};

public class GeneticAlgorithmCS : MonoBehaviour {

	public ComputeShader shader;
	private ComputeBuffer testing;
	private int populationSize = 128 * 128;

	uint NumberOfSetBits(uint i)
	{
		i = i - ((i >> 1) & 0x55555555);
		i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
		return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
	}

	// Use this for initialization
	void Start () {
		Debug.Log("Population size: " + populationSize);
		//int width = (int)Mathf.Round(Mathf.Sqrt(populationSize));
		//int height = (int)Mathf.Round(Mathf.Sqrt(populationSize));

		testing = new ComputeBuffer(10, Marshal.SizeOf(typeof(Individual)));

		Debug.Log("Seed " + DateTime.Now.Millisecond);

		// Fill with random genome, and run first fitness test.
		int kernel = shader.FindKernel("InitializePopulation");
		DebugAux.Assert(kernel >= 0, "Couldn't find kernel: " + "InitializePopulation " + kernel);
		shader.SetBuffer(kernel, "Population", testing);
		shader.SetFloat("seed", DateTime.Now.Millisecond);
		shader.Dispatch(kernel, 32, 32, 1);

		Individual[] tes = new Individual[10];
		testing.GetData(tes);
		for (int i = 0; i < tes.Length; i++)
			Debug.Log(tes[i].genome + " " + tes[i].fitness);

		// Selection..
		/*kernel = shader.FindKernel("AllOnesFitness");
		DebugAux.Assert(kernel >= 0, "Couldn't find kernel: " + "AllOnesFitness " + kernel);
		shader.SetBuffer(kernel, "Population", testing);
		shader.Dispatch(kernel, 32, 32, 1);*/

		testing.Dispose();
	}
	
	void OnGUI() {
		// GUI.DrawTexture(new Rect(0, 0, 100, 100), testing);
	}

	void Update() {

	}
}
