using UnityEngine;
using System.Collections.Generic;

public class GeneticAlgorithm : MonoBehaviour {
	public delegate void NextStepDelegate();

	private enum State {
		FitnessTest = 0,
		Selection = 1,
		Mating = 2,
		Mutation = 3,
		NrStates = 4,
	}

	public AEnvironment environment;
	public ASelectionStrategy selectionStrategy;
	public AMatingStrategy matingStrategy;
	public AMutationStrategy mutationStrategy;
	public int nrTimes = 10;
	
	public bool isPaused = true;

	public GenomeType genomeType;
	public int genomeSize;

	private bool blocking = false;
	private int populationSize;
	private int nrGenerations;

	private Population population;
	private SelectionBuffer selectionBuffer;

	private NextStepDelegate nextStepDelegate;

	private int nrIterations = 0;
	private bool isDone = false;
	private bool isWaiting = false;
	private State currentState;
	private float totalDuration;

	private GASettings settings;
	private Statistics statistics;

	// Adaptive GAs
	// In CAGA (clustering-based adaptive genetic algorithm)
	void Start() {
		settings = GetComponent<GASettings>();
		populationSize = settings.PopulationSize;
		nrGenerations = settings.NumberOfGenerations;
		blocking = settings.RunAsFastAsPossible;

		environment.Settings = settings;
		selectionStrategy.Settings = settings;
		matingStrategy.Settings = settings;
		mutationStrategy.Settings = settings;

		population = new Population(populationSize);
		selectionBuffer = new SelectionBuffer(populationSize);

		statistics = GetComponent<Statistics>();
		if (statistics == null)
			statistics = gameObject.AddComponent<Statistics>();

		statistics.Population = population;

		BaseGenome genomeTemplate = GenomeFactory.CreateGenome(genomeType, genomeSize);
		for (int i = 0; i < populationSize; i++) {
			BaseGenome genome = genomeTemplate.CreateRandom(); // Should be possible to generate these by hand (seeded)
			population[i] = new PhenomeDescription(genome);
		}
		for (int i = 0; i < populationSize; i++) {
			BaseGenome genome = genomeTemplate.CreateRandom(); // Should be possible to generate these by hand (seeded)
			selectionBuffer[i] = genome;
		}

		nextStepDelegate = NextStep;
		currentState = State.FitnessTest;
		environment.Inititalize(population);
	}

	public void Update() {
		if (isPaused || isDone)
			return;

		BasePhenome.NrTimes = nrTimes;

		while (!isWaiting) {
			statistics.OnStepBegin();
			Step();
			statistics.OnStepEnd();
			if (isDone || !blocking)
				break;
		}

		if (isDone)
			statistics.OnDone();
	}

	/*A solution is found that satisfies minimum criteria
		Fixed number of generations reached
		Allocated budget (computation time/money) reached
		The highest ranking solution's fitness is reaching or has reached a plateau such that successive iterations no longer produce better results
		Manual inspection
		Combinations of the above*/
	public bool IsDone() {
		return nrIterations >= nrGenerations || 
			(settings.TerminateSolutionFound && population.MaxFitness >= settings.WantedFitness);
	}

	public void Step() {
		isWaiting = true;
		if (currentState == State.FitnessTest) {
			environment.FitnessFunction(population, nextStepDelegate); // This test is called the Objective Function, or a Fitness Function
			nrIterations++;
		}

		isDone = IsDone();
		if (isDone)
			return;

		if (currentState == State.Selection)
			selectionStrategy.Select(population, selectionBuffer, nextStepDelegate);

		// These are genetic operators, there can be any number of them.. How to extend??
		// it is possible to use other operators such as regrouping, colonization-extinction, or migration in genetic algorithms
		// parameters such as the mutation probability, crossover probability
		if (currentState == State.Mating)
			matingStrategy.Mate(selectionBuffer, population, nextStepDelegate); // Usually called crossover
		if (currentState == State.Mutation)
			mutationStrategy.Mutate(population, nextStepDelegate);
		// increasing the probability of mutation when the solution quality drops (called triggered hypermutation
		// occasionally introducing entirely new, randomly generated elements into the gene pool (called random immigrants
	}

	public void NextStep() {
		int s = (int)currentState;
		s++;
		s %= (int)(State.NrStates);
		currentState = (State) s;
		isWaiting = false;
	}

	public void OnGUI() {
		GUI.Label(new Rect(200, 200, 200, 200), "" + nrIterations);
		
		if (GUI.Button(new Rect(50, 50, 200, 50), isPaused ? "Play" : "Pause"))
			isPaused = !isPaused;
	}
}
