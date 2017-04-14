using UnityEngine;
using System.Collections;
using System;

public static class DebugAux
{
	public static void Assert(bool iCondition, string iErrorMessage)
	{
		if (!iCondition) {
			Debug.LogError(iErrorMessage);
			Debug.Break();
		}
	}
}

