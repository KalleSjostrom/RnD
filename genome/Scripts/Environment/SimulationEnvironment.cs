using UnityEngine;
using System.Collections;

public abstract class SimulationEnvironment : AEnvironment {

	public GameObject phenomePrefab;

	private bool simulating = false;
	protected BasePhenome[] phenomeBuffer;
	protected Population population;
	protected GeneticAlgorithm.NextStepDelegate doneCallback;

	public override void Inititalize(Population population) {
		if (phenomeBuffer == null) {
			phenomeBuffer = new BasePhenome[population.Size];
			for (int i = 0; i < population.Size; i++) {
				phenomeBuffer[i] = Helper.InstansiateAndGet<BasePhenome>(phenomePrefab);
			}
		}
	}

	public override void FitnessFunction(Population population, GeneticAlgorithm.NextStepDelegate callback) {
		this.population = population;

		for (int i = 0; i < population.Size; i++) {
			PhenomeDescription pd = population[i];
			phenomeBuffer[i].Init(pd.Genome);
		}

		InitiateNewSimulation();

		simulating = true;
		doneCallback = callback;
	}

 	protected virtual void InitiateNewSimulation() {}

	private void Update() {
		if (!simulating)
			return;

		bool everyoneDone = true;
		for (int i = 0; i < phenomeBuffer.Length; i++) {
			everyoneDone = everyoneDone && phenomeBuffer[i].IsDone();
			if (!everyoneDone)
				break;
		}

		if (everyoneDone) {
			simulating = false;
			for (int i = 0; i < population.Size; i++) {
				PhenomeDescription pd = population[i];
				pd.Fitness = CalculateFitness(phenomeBuffer[i]);
			}
			
			CalculateMinMaxTotal(population);
			Debug.Log(population.MaxFitness + " " + population.MinFitness + " " + population.TotalFitness);
			doneCallback();
		}
	}

	protected abstract float CalculateFitness(BasePhenome phenome);
}
