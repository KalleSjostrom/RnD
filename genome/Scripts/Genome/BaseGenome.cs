using UnityEngine;
using System.Collections;

public abstract class BaseGenome {

	public virtual int Length { get; set; }

	public abstract BaseGenome CreateRandom();
	public abstract void CloneFrom(BaseGenome source);

	// Mating or crossovers
	public abstract void OnePointCrossover(BaseGenome mom, BaseGenome dad, int point);

	// Mutations
	public abstract void Modify(int point);
	public abstract void Switch(int point1, int point2);
}
