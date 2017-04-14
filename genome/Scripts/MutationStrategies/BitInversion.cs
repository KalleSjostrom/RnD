using UnityEngine;
using System.Collections;

public class BitInversion : AMutationStrategy {

	public float mutationProbability = 0.1f;
	public float bitMutationFraction = 0.2f;

	public override void Mutate(Population population, GeneticAlgorithm.NextStepDelegate callback) {
		for (int i = 0; i < population.Size; i++) {
			if (Random.value > mutationProbability)
				continue;

			BaseGenome genome = population[i].Genome;
			int nrMutations = Random.Range(0, (int)Mathf.Round(bitMutationFraction * genome.Length));
			for (int j = 0; j < nrMutations; j++) {
				genome.Modify(Random.Range(0, genome.Length));
			}
		}
		callback();
	}

	/*
	 * Bit string mutation
The mutation of bit strings ensue through bit flips at random positions.
Example:
1	0	1	0	0	1	0
↓		
1	0	1	0	1	1	0
The probability of a mutation of a bit is \frac{1}{l}, where l is the length of the binary vector. Thus, a mutation rate of 1 per mutation and individual selected for mutation is reached.
Flip Bit
This mutation operator takes the chosen genome and inverts the bits. (i.e. if the genome bit is 1,it is changed to 0 and vice versa)
Boundary
This mutation operator replaces the genome with either lower or upper bound randomly. This can be used for integer and float genes.
Non-Uniform
The probability that amount of mutation will go to 0 with the next generation is increased by using non-uniform mutation operator.It keeps the population from stagnating in the early stages of the evolution.It tunes solution in later stages of evolution.This mutation operator can only be used for integer and float genes.
Uniform
This operator replaces the value of the chosen gene with a uniform random value selected between the user-specified upper and lower bounds for that gene. This mutation operator can only be used for integer and float genes.
Gaussian
This operator adds a unit Gaussian distributed random value to the chosen gene. If it falls outside of the user-specified lower or upper bounds for that gene,the new gene value is clipped. This mutation operator can only be used for integer and float genes.
*/
}
