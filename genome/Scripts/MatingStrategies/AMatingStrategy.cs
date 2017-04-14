using UnityEngine;
using System.Collections;

public abstract class AMatingStrategy : MonoBehaviour {

	public GASettings Settings { get; set; }

	// Should produce two children!
	public abstract void Mate(SelectionBuffer selected, Population newPopulation, GeneticAlgorithm.NextStepDelegate callback);

	/*One-point crossover[edit]
	A single crossover point on both parents' organism strings is selected. All data beyond that point in either organism string is swapped between the two parent organisms. The resulting organisms are the children:*/

	/*Two-point crossover[edit]
Two-point crossover calls for two points to be selected on the parent organism strings. Everything between the two points is swapped between the parent organisms, rendering two child organisms:
*/

	/*
	 * "Cut and splice"[edit]
Another crossover variant, the "cut and splice" approach, results in a change in length of the children strings. The reason for this difference is that each parent string has a separate choice of crossover point.
*/

	/*
	 * Uniform Crossover and Half Uniform Crossover[edit]
The Uniform Crossover uses a fixed mixing ratio between two parents. Unlike one- and two-point crossover, the Uniform Crossover enables the parent chromosomes to contribute the gene level rather than the segment level.
If the mixing ratio is 0.5, the offspring has approximately half of the genes from first parent and the other half from second parent, although cross over points can be randomly chosen as seen below:

The Uniform Crossover evaluates each bit in the parent strings for exchange with a probability of 0.5. Even though the uniform crossover is a poor method, empirical evidence suggest that it is a more exploratory approach to crossover than the traditional exploitative approach that maintains longer schemata. This results in a more complete search of the design space with maintaining the exchange of good information. Unfortunately, no satisfactory theory exists to explain the discrepancies between the Uniform Crossover and the traditional approaches. [2]
In the uniform crossover scheme (UX) individual bits in the string are compared between two parents. The bits are swapped with a fixed probability, typically 0.5.
In the half uniform crossover scheme (HUX), exactly half of the nonmatching bits are swapped. Thus first the Hamming distance (the number of differing bits) is calculated. This number is divided by two. The resulting number is how many of the bits that do not match between the two parents will be swapped.
*/

	/*
	 * Three parent crossover[edit]
[icon]	This section requires expansion. (June 2013)
In this technique, the child is derived from three parents. They are randomly chosen. Each bit of first parent is checked with bit of second parent whether they are same. If same then the bit is taken for the offspring otherwise the bit from the third parent is taken for the offspring. For example, the following three parents:
parent1 1 1 0 1 0 0 0 1 0
parent2 0 1 1 0 0 1 0 0 1
parent3 1 1 0 1 1 0 1 0 1
produces the following offspring:
offspring 1 1 0 1 0 0 0 0 1[3]*/

	/*
	 * Crossover for Ordered Chromosomes[edit]
Depending on how the chromosome represents the solution, a direct swap may not be possible. One such case is when the chromosome is an ordered list, such as an ordered list of the cities to be travelled for the traveling salesman problem. There are many crossover methods for ordered chromosomes. The already mentioned N-point crossover can be applied for ordered chromosomes also, but this always need a corresponding repair process, actually, some ordered crossover methods are derived from the idea. However, sometimes a crossover of chromosomes produces recombinations which violate the constraint of ordering and thus need to be repaired. Several examples for crossover operators (also mutation operator) preserving a given order are given in:[4]
partially matched crossover (PMX): In this method, two crossover points are selected at random and PMX proceeds by position wise exchanges. The two crossover points give matching selection. It affects cross by position-by-position exchange operations. In this method parents are mapped to each other, hence we can also call it partially mapped crossover.[5]
cycle crossover (CX): Beginning at any gene i in parent 1, the i-th gene in parent 2 becomes replaced by it. The same is repeated for the displaced gene until the gene which is equal to the first inserted gene becomes replaced (cycle).
order crossover operator (OX1): A portion of one parent is mapped to a portion of the other parent. From the replaced portion on, the rest is filled up by the remaining genes, where already present genes are omitted and the order is preserved.
order-based crossover operator (OX2)
position-based crossover operator (POS)
voting recombination crossover operator (VR)
alternating-position crossover operator (AP)
sequential constructive crossover operator (SCX)[6]
Other possible methods include the edge recombination operator.
*/
}
