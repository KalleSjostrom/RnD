using UnityEngine;
using System.Collections;

public abstract class ASelectionStrategy : MonoBehaviour {

	public GASettings Settings { get; set; }

	// rate only a random sample of the population, as the former process may be very time-consuming.
	public abstract void Select(Population population, SelectionBuffer selected, GeneticAlgorithm.NextStepDelegate callback);
	//A very successful (slight) variant of the general process of constructing a new population is to allow some of the better organisms from the current generation to carry over to the next, unaltered. This strategy is known as elitist selection.

	/*
	 * Roulette wheel selection (SCX)[1] It is also known as fitness proportionate selection. The individual is selected on the basis of fitness. The probability of an individual to be selected increases with the fitness of the individual greater or less than its competitor's fitness.
Boltzmann selection
Tournament selection
Rank selection
Steady state selection
Truncation selection
Local selection*/
}
