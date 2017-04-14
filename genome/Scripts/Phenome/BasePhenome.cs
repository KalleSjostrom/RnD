using UnityEngine;
using System.Collections;

public abstract class BasePhenome : MonoBehaviour {
	public static int NrTimes = 10;
	private Vector3 startPosition;
	private Quaternion startRotation;

	private bool isDone;
	private bool initialized = false;
	private float duration;

	void Start() {
		startRotation = gameObject.transform.rotation;
		startPosition = gameObject.transform.position;
	}

	public virtual void Init(BaseGenome genome) {
		gameObject.transform.rotation = startRotation;
		gameObject.transform.position = startPosition;

		duration = 0;
		isDone = false;
		initialized = true;
	}

	void FixedUpdate() {
		if (!initialized)
			return;

		for (int i = 0; i < BasePhenome.NrTimes; i++) {
			duration += Time.fixedDeltaTime;
			UpdatePhenome();
			if (isDone)
				break;
		}
	}

	protected void SetAsDone() {
		isDone = true;
		initialized = false;
	}
	public bool IsDone() {
		return isDone;
	}

	protected abstract void UpdatePhenome();
}