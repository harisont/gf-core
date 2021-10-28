package org.grammaticalframework.pgf;

import java.util.Arrays;
import java.util.List;

/** A class for types in the abstract syntax of a grammar */
public class Type {

	public Type(Hypo[] hypos, String cat, Expr[] exprs) {
		this.hypos = hypos;
		this.cat = cat;
		this.exprs = exprs;
	}

	public Type(Hypo[] hypos, String cat) {
		this.hypos = hypos;
		this.cat = cat;
		this.exprs = new Expr[0];
	}

	public Type(String cat, Expr[] exprs) {
		this.hypos = new Hypo[0];
		this.cat = cat;
		this.exprs = exprs;
	}

	public Type(String cat) {
		this.hypos = new Hypo[0];
		this.cat = cat;
		this.exprs = new Expr[0];
	}

	/**
	 * Returns the category of the type.
	 * @return The category of the type.
	 */
	public String getCategory() {
		return this.cat;
	}
	
	/**
	 * Returns an array of arguments for the category.
	 * @return An array of arguments for the category.
	 */
	public Expr[] getExprs() {
		return this.exprs;
	}
	
	/**
	 * Returns an array of hypotheses if the type is a function type. 
	 * If the represented type is A1 -&gt; A2 -&gt; ... An -&gt; B, then
	 * the hypotheses represent the types in A1, A2 ... An.
	 * @return An array of hypotheses if this is a function type.
	 */
	public Hypo[] getHypos() {
		return this.hypos;
	}
	
	/**
	 * Reads a type from the given string.
	 * @param s The string containing the name of the type to read.
	 * @return The corresponding `Type` to read.
	 */
	public native static Type readType(String s);

	/**
	 * Convert a `Type` to `String`.
	 * @return The string equivalent of the Type, excluding any variables in 
	 * it.
	 */
	@Override
	public String toString() {
		return toStringContext(new String[0]);
	}

	/**
	 * Convert a `Type` to `String`.
	 * @return The string equivalent of the Type.
	 */
	public native String toStringContext(String[] vars);
	
	@Override
    public boolean equals(Object o) {
        if (o == this) return true; // same object
        if (!(o instanceof Type)) return false;
		Type t = (Type)o;
        return Arrays.equals(this.hypos, t.hypos) && this.cat.equals(t.cat) && Arrays.equals(this.exprs, t.exprs);
	}

	//////////////////////////////////////////////////////////////////
	// private stuff

	private Hypo[] hypos;
	private String cat;
	private Expr[] exprs;
}
