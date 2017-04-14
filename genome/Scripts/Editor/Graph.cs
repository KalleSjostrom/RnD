using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Collections.Generic;

namespace AGP {
	public class Graph {

		private List<Node> nodes;
		private List<Edge> edges;

		public Graph() {
			nodes = new List<Node>();
			edges = new List<Edge>();
		}

		public void AddNode(Node n) {
			nodes.Add(n);
		}
		public void RemoveNode(Node n) {
			nodes.Remove(n);
		}

		public void AddEdge(Edge e) {
			edges.Add(e);
		}
		public void RemoveEdge(Edge e) {
			edges.Remove(e);
		}

		public void Draw(EditorWindow main, GAEditor editor) {
			foreach (Node n in nodes)
				n.Draw(main, editor);

			foreach (Edge e in edges)
				e.Draw(main, editor);
		}
	}
}