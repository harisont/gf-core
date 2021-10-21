package org.grammaticalframework.pgf;

public class Hypo {
	
	/**
	 * Returns the hypothesis's bind type (`true` if explicit, 
	 * `false` if implicit).
	 * @return The hypothesis's bind type.
	 */
	public boolean getBindType() {
		return this.bindType;
	}
	
	/**
	 * Returns the name of the bound variable (`_` if there is none).
	 * @return The name of the bound variable.
	 */
	public String getVar() {
		return this.var;
	};
	
	/**
	 * Returns the type for the hypothesis.
	 * @return The type for the hypothesis.
	 */
	public Type getType() {
		return this.type;
	}

	@Override
    public boolean equals(Object o) {
		if (o == this) return true; // same object
        if (!(o instanceof Hypo)) return false;
		Hypo h = (Hypo)o;
        return this.bindType == h.bindType && this.var.equals(h.var) && this.type.equals(h.type);
	}

	//////////////////////////////////////////////////////////////////
	// private stuff

	private boolean bindType;
	private String var;
	private Type type;

	private Hypo(boolean bindType, String var, Type type) {
		this.bindType = bindType;
		this.var = var;
		this.type = type;
	}
}
