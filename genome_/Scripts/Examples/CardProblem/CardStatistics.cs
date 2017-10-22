using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class CardStatistics : Statistics {

	private List<float> avg = new List<float>();

	public override void OnStepBegin() {
		base.OnStepBegin();
	}
	public override void OnStepEnd() {
		base.OnStepEnd();
		avg.Add(Population.MaxFitness);
	}

	public override void OnDone() {
		for (int i = 0; i < avg.Count; i++) {
			float mean = avg[i];
			Debug.Log(i + " " + mean);
		}
		/*
		List<PhenomeDescription> list = new List<PhenomeDescription>();
		for (int i = 0; i < Population.Size; i++)
			list.Add(Population[i]);
		
		list.Sort();
		for (int i = 0; i < Mathf.Min(5, Population.Size); i++) {
			string adds = "";
			string muls = "";
			PhenomeDescription desc = list[i];
			BaseGenomeBinary genome = list[i].Genome as BaseGenomeBinary;
			
			// int fitness = CalculateFitness(genome);
			// DebugAux.Assert(fitness == desc.Fitness, "Fitness mismatch: " + fitness + " " + desc.Fitness + " " + genome.ToString());
			
			for (int k = 0; k < genome.Length; k++) {
				if (genome.IsSet(k)) {
					adds += (k+1) + " ";
				} else {
					muls += (k+1) + " ";
				}
			}
			Debug.Log("Fitness " + list[i].Fitness + " Adds: " + adds + " Muls: " + muls);
		}
		Debug.Log("Total Duration: " + Duration);*/
	}
}
