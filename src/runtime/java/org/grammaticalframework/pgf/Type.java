package org.grammaticalframework.pgf;

/** A class for types in the abstract syntax of a grammar */
public class Type {

	/** The category */
	public native String getCategory();
	
	/** An array of arguments for the category */
	//public native Expr[] getExprs();
	
	/** An array of hypotheses if this is a function type. 
	 * If the represented type is A1 -&gt; A2 -&gt; ... An -&gt; B, then
	 * the hypotheses represent the types in A1, A2 ... An. */
	//public native Hypo[] getHypos();
	
	/**
	 * Reads a type from the given string.
	 * @param s The string containing the name of the type to read.
	 * @return The corresponding `Type` to read.
	 */
	public native static Type readType(String s);

	@Override
	public native String toString();
	
	@Override
    public boolean equals(Object o) {
        if (o == this) return true; // same object
        if (!(o instanceof Type)) return false;
		Type t = (Type)o;
        return this.hypos.equals(t.hypos) && this.cat.equals(t.cat) && this.exprs.equals(t.exprs);
	}

	//////////////////////////////////////////////////////////////////
	// private stuff

	private Hypo[] hypos;
	private String cat;
	private Expr[] exprs;

	private Type(Hypo[] hypos, String cat, Expr[] exprs) {
		this.hypos = hypos;
		this.cat = cat;
		this.exprs = exprs;
	}
}
