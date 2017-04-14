using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System;

public class Statistics : MonoBehaviour {

	public Population Population { get; set; }

	// TODO: Move the logging-specific stuff to own component.
	public bool logToFile = false;
	public string logFileName = "statistics";

	public StreamWriter Log { get { return logFile; } }

	protected float duration = 0;

	private StreamWriter logFile;
	private float start;

	public void Start() {
		if (logToFile) {
			string fileName = logFileName + DateTime.Now.ToString("_MM-dd-yyyy_HH-mm-ss-FFF") + ".txt";
			logFile = File.CreateText(fileName);
			logFile.AutoFlush = true;
		}
	}
	
	public virtual void OnStepBegin() {
		start = Time.realtimeSinceStartup;
	}
	public virtual void OnStepEnd() {
		duration += Time.realtimeSinceStartup - start;
	}

	public virtual void OnDone() {}
}
