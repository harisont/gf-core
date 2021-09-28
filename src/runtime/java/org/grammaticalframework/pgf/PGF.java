package org.grammaticalframework.pgf;

import java.io.*;
import java.nio.file.FileAlreadyExistsException;
import java.util.*;

/** This is the class for PGF grammars.*/
public class PGF {
	/** Reads a grammar in PGF format.
	 * @param path The path to the PGF file.
	 * @return a PGF object representing the grammar in memory. */
	public static native PGF readPGF(String path) throws FileNotFoundException;

	/** Reads a grammar in PGF format and stores the unpacked data in a 
	 * platform-dependent NGF file, ready to be shared with other process, 
	 * or used for quick re-start.
	 * @param pgfPath The path to the PGF file.
	 * @param ngfPath The desired path for the new NGF file
	 * @return a PGF object representing the grammar in memory. */
	public static native PGF bootNGF(String pgfPath, String ngfPath) 
	throws FileNotFoundException, FileAlreadyExistsException;

	/** Reads a grammar in NGF format. 
	 * @param path The path to the NGF file.
	 * @return a PGF object representing the grammar in memory.*/
	public static native PGF readNGF(String path) throws FileNotFoundException;

	/**
	 * Creates an empty grammar with the given abstract name and stores it
	 * on disk at the given path.
	 * @param abstractName The name of the empty grammar to create.
	 * @param path The path the grammar will be stored at.
	 * @return
	 */
	public static native PGF newNGF(String abstractName, String path) 
	throws FileAlreadyExistsException;

	/**
	 * Creates an empty grammar with the given abstract name, storing it only
	 * in memory.
	 * @param abstractName The name of the empty grammar to create.
	 * @return
	 */
	public static native PGF newNGF(String abstractName);

	/** Reads a grammar from an input stream.
	 * @param stream The stream from which to read the grammar
	 * @return an object representing the grammar in memory. */
	public static native PGF readPGF(InputStream stream);

	/** Returns the name of the abstract syntax for the grammar */
	public native String getAbstractName();

	/** Returns a map from a name of a concrete syntax to 
	 * a {@link Concr} object for the syntax. */
	public native Map<String,Concr> getLanguages();

	/** Returns a list of with all categories in the grammar */
	public native List<String> getCategories();
	
	/** The name of the start category for the grammar. This is usually
	 * specified with 'params startcat=&lt;cat&gt;'.
	 */
	public native String getStartCat();
	
	/** Returns a list with all functions in the grammar. */
	public native List<String> getFunctions();
	
	/** Returns a list with all functions with a given return category.
	 * @param cat The name of the return category. */
	public native List<String> getFunctionsByCat(String cat);
	
	/** Returns the type of the function with the given name.
	 * @param fun The name of the function.
	 */
	public native Type getFunctionType(String fun);

	/** Returns the negative logarithmic probability of the function
	 * with the given name.
	 * @param fun The name of the function.
	 */
	public native double getFunctionProb(String fun);

	/** Returns an iterable over the set of all expression in
	 * the given category. The expressions are enumerated in decreasing
	 * probability order.
	 */
	public Iterable<ExprProb> generateAll(String startCat) {
		return new Generator(this, startCat);
	}

	/** Normalizes an expression to its normal form by using the 'def'
	 * rules in the grammar.
	 * 
	 * @param expr the original expression.
	 * @return the normalized expression.
	 */
	public native Expr compute(Expr expr);

	/** Takes an expression and returns a refined version
	 * of the expression together with its type */
	public native TypedExpr inferExpr(Expr expr) throws TypeError;
	
	/** Takes an expression and checks it agains a type. The returned expression
	 * is a possibly refined version of the original. */
	public native Expr checkExpr(Expr expr, Type ty) throws TypeError;

	public native String graphvizAbstractTree(Expr expr);

	@Override
	public native void finalize();

	//////////////////////////////////////////////////////////////////
	// private stuff
	private long db;
	private long rev;

	private PGF(long db, long rev) {
		this.db = db;
		this.rev = rev;
	}
	
	static { 
         System.loadLibrary("jpgf");
    }
}
