using UnityEngine;
using System.Collections;
using System;

public class Helper
{
	public static T CreateObject<T>(String iName) where T : UnityEngine.Component
	{
		GameObject go = new GameObject(iName, typeof(T));
		return go.GetComponent<T>();
	}
	
	public static T CreateObject<T>(String iName, GameObject iParent) where T : UnityEngine.Component
	{
		GameObject go = new GameObject(iName, typeof(T));
		go.transform.parent = iParent.transform;
		return go.GetComponent<T>();
	}
	
	public static T Instansiate<T>(GameObject iPrefab) where T : UnityEngine.Component
	{
		GameObject go = GameObject.Instantiate(iPrefab) as GameObject;
		go.AddComponent<T>();
		return go.GetComponent<T>();
	}
	
	public static T InstansiateAndGet<T>(GameObject iPrefab) where T : UnityEngine.Component
	{
		GameObject go = GameObject.Instantiate(iPrefab) as GameObject;
		return go.GetComponent<T>();
	}

	public static T InstansiateAndGet<T>(GameObject iPrefab, GameObject iParent) where T : UnityEngine.Component
	{
		GameObject go = GameObject.Instantiate(iPrefab) as GameObject;
		go.transform.parent = iParent.transform;
		return go.GetComponent<T>();
	}
	
	public static T Instansiate<T>(GameObject iPrefab, GameObject iParent) where T : UnityEngine.Component
	{
		GameObject go = GameObject.Instantiate(iPrefab) as GameObject;
		go.AddComponent<T>();
		go.transform.parent = iParent.transform;
		return go.GetComponent<T>();
	}
	
	public static T Find<T>(String iName) where T : UnityEngine.Component
	{
		GameObject go = GameObject.Find(iName) as GameObject;
		return go.GetComponent<T>();
	}
	
	public static T FindAndAdd<T>(String iName) where T : UnityEngine.Component
	{
		GameObject go = GameObject.Find(iName) as GameObject;
		return go.AddComponent<T>();
	}
}
