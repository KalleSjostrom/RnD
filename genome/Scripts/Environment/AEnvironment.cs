using UnityEngine;
using System.Collections;

public abstract class AEnvironment : MonoBehaviour {

	public GASettings Settings { get; set; }

	public virtual void Inititalize(Population population) {}
	public abstract void FitnessFunction(Population population, GeneticAlgorithm.NextStepDelegate callback);

	public virtual void FitnessFunctionSimple(Population population, GeneticAlgorithm.NextStepDelegate callback) {
		for (int i = 0; i < population.Size; i++) {
			PhenomeDescription pd = population[i];
			pd.Fitness = CalculateFitness(pd.Genome);
		}
		
		CalculateMinMaxTotal(population);
		callback();
	}

	public virtual float CalculateFitness(BaseGenome genome) { return 0; }

	// TODO: One calculate function with bitmask.
	public void CalculateMinMax(Population population) {
		float minFitness = Mathf.Infinity;
		float maxFitness = -Mathf.Infinity;
		
		for (int i = 0; i < population.Size; i++) {
			PhenomeDescription pd = population[i];
			maxFitness = Mathf.Max(maxFitness, pd.Fitness);
			minFitness = Mathf.Min(minFitness, pd.Fitness);
		}
		
		population.MinFitness = minFitness;
		population.MaxFitness = maxFitness;
	}

	public void CalculateMinMaxTotal(Population population) {
		float minFitness = Mathf.Infinity;
		float maxFitness = -Mathf.Infinity;
		float total = 0;
		
		for (int i = 0; i < population.Size; i++) {
			PhenomeDescription pd = population[i];
			float f = pd.Fitness;
			maxFitness = Mathf.Max(maxFitness, f);
			minFitness = Mathf.Min(minFitness, f);
			total += f;
		}
		
		population.MinFitness = minFitness;
		population.MaxFitness = maxFitness;
		population.TotalFitness = total;
	}

	public void CalculateMedian(Population population) {
		population.Sort();
		int middleIndex = population.Size/2;
		population.MedianFitness = population[middleIndex].Fitness;
	}
}
