package org.grammaticalframework.pgf;

public class Hypo {
	public native boolean getBindType();
	
	/** The name of the bound variable or '_' if there is none */
	public native String getVar();
	
	/** The type for this hypothesis */
	public native Type getType();

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
