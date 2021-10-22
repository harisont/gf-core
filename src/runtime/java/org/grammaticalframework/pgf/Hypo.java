package org.grammaticalframework.pgf;

public class Hypo {
	
	/**
	 * Basically a constructor of `Hypo`s for non-dependent `Type`s, i.e. `A`,
	 * taking only the `Type` itself as parameter.
	 * It sets `bindType` to `true` and `var` to `_`.
	 * @param type A `Type`.
	 * @return A `Hypo` for the non-dependent type `type`.
	 */
	public static Hypo mkHypo(Type type) {
		boolean bt = true;
		String v = "_";
		Type t = type;
		return new Hypo(bt, v, t);
	}

	/**
	 * Basically a constructor of `Hypo`s for dependent types, i.e. `x : A`.
	 * It sets `bindType` to `true`.
	 * @param var A variable name.
	 * @param type A `Type`.
	 * @return A `Hypo` for the dependent type `type`.
	 */
	public static Hypo mkDepHypo(String var, Type type) {
		boolean bt = true;
		String v = var;
		Type t = type;
		return new Hypo(bt, v, t);
	}

	/**
	 * Basically a constructor of `Hypo`s for dependent types with implicit
	 * arguments, i.e. `{x} : A`. It sets `bindType` to `false`.
	 * @param var A variable name.
	 * @param type A `Type`.
	 * @return A `Hypo` for the dependent type `type`.
	 */
	public static Hypo mkImplHypo(String var, Type type) {
		boolean bt = false;
		String v = var;
		Type t = type;
		return new Hypo(bt, v, t);
	}
	
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
