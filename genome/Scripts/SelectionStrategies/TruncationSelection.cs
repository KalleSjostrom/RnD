using UnityEngine;
using System.Collections;

public class TruncationSelection : ASelectionStrategy {

	public enum TruncationMode {
		AboveMiddle,
		AboveMean,
		AboveMedian,
	}

	public TruncationMode truncationMode;
	
	public override void Select(Population population, SelectionBuffer selection, GeneticAlgorithm.NextStepDelegate callback) {
		int size = 0;

		// TODO: Implement settings for how to truncate.
		float cutoff = 0;
		switch (truncationMode) {
		case TruncationMode.AboveMiddle:
			cutoff = population.MinFitness + (population.MaxFitness - population.MinFitness)/2;
			break;
		case TruncationMode.AboveMean:
			cutoff = population.MeanFitness;
			break;
		case TruncationMode.AboveMedian:
			cutoff = population.MedianFitness;
			break;
		}

		for (int i = 0; i < population.Size; i++) {
			if (population[i].Fitness > cutoff) {
				selection[size].CloneFrom(population[i].Genome);
				size++;
			}
		}

		selection.Size = size;
		callback();
	}
}
