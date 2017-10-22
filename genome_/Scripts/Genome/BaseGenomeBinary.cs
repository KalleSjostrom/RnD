using UnityEngine;
using System.Collections;

public abstract class BaseGenomeBinary : BaseGenome {
	public abstract void Set(int index, bool isSet);
	public abstract bool IsSet(int index);
	public abstract void Flip(int index);
}
