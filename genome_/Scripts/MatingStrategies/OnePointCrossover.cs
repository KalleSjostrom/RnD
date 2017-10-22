using UnityEngine;
using System.Collections;

// One-point crossover[edit]
// A single crossover point on both parents' organism strings is selected.
// All data beyond that point in either organism string is swapped between the two parent organisms.
// The resulting organisms are the children
public class OnePointCrossover : AMatingStrategy {

	public override void Mate(SelectionBuffer selected, Population newPopulation, GeneticAlgorithm.NextStepDelegate callback) {
		// The new population is populated with 2 children each iteration.
		for (int i = 0; i < newPopulation.Size; i+=2) {
			BaseGenome mom = selected[Random.Range(0, selected.Size)]; // TODO: mom and dad can be the same..
			BaseGenome dad = selected[Random.Range(0, selected.Size)];

			int point = Random.Range(0, mom.Length);
			newPopulation[i].Genome.OnePointCrossover(mom, dad, point);
			newPopulation[i+1].Genome.OnePointCrossover(mom, dad, point);
		}
		callback();
	}
}
