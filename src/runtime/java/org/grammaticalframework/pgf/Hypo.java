package org.grammaticalframework.pgf;

public class Hypo {
	public boolean getBindType() {
		return this.bindType;
	}
	
	/** The name of the bound variable or '_' if there is none */
	public String getVar() {
		return this.var;
	};
	
	/** The type for this hypothesis */
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
