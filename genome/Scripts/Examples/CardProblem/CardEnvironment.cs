using UnityEngine;
using System.Collections;

public class CardEnvironment : AEnvironment {

	public override void FitnessFunction(Population population, GeneticAlgorithm.NextStepDelegate callback) {
		for (int i = 0; i < population.Size; i++) {
			PhenomeDescription pd = population[i];
			pd.Fitness = CalculateFitness(pd.Genome as BaseGenomeBinary);
		}

		CalculateMinMaxTotal(population);
		callback();
	}

	public float CalculateFitness(BaseGenomeBinary genome) {
		int leftPile = 0;
		int rightPile = 1;
		for (int i = 0; i < genome.Length; i++) {
			if (genome.IsSet(i)) {
				leftPile += (i+1);
			} else {
				rightPile *= (i+1);
			}
		}
		int leftError = Mathf.Abs(36-leftPile);
		int rightError = Mathf.Abs(360-rightPile);
		// The max value of the right pile is: 3628800.
		// Hence the maximum error is: 36 + 3628800/10 = 362916
		return 1-(leftError + rightError/10.0f)/362916.0f;
	}
}
