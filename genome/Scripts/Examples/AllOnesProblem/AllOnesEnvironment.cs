using UnityEngine;
using System.Collections;

public class AllOnesEnvironment : AEnvironment {

	public override void FitnessFunction(Population population, GeneticAlgorithm.NextStepDelegate callback) {
		FitnessFunctionSimple(population, callback);
	}
	
	public override float CalculateFitness(BaseGenome genome) {
		BaseGenomeBinary g = genome as BaseGenomeBinary;
		int nrOnes = 0;
		for (int i = 0; i < g.Length; i++)
			nrOnes += g.IsSet(i) ? 1 : 0;
		return nrOnes;
	}
}
