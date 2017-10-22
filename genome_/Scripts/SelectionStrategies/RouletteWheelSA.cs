using UnityEngine;
using System.Collections;

// Based on the paper: "Roulette-wheel selection via stochastic acceptance" by Adam Lipowski and Dorota Lipowska.
// http://arxiv.org/abs/1109.3627
// Complexity: O(1).
public class RouletteWheelSA : ASelectionStrategy {
	
	public override void Select(Population population, SelectionBuffer selection, GeneticAlgorithm.NextStepDelegate callback) {
		int size = 0;
		DebugAux.Assert(population.MaxFitness != 0, "[RouletteWheelSA] Can't have a MaxFitness of zero!");

		// The algorithm states that we should pick one at random for consideration with probability 1/N.
		int nrConsiderations = Mathf.RoundToInt(population.Size * Settings.SelectionProportion);
		for (int i = 0; i < nrConsiderations; i++) {
			int index = Random.Range(0, population.Size);
			float probability = population[index].Fitness / population.MaxFitness;
			if (Random.value <= probability) {
				selection[size].CloneFrom(population[index].Genome);
				size++;
			}
		}

		selection.Size = size;
		callback();
	}
}
