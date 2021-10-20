package org.grammaticalframework.pgf;

import java.io.*;
import java.nio.file.FileAlreadyExistsException;
import java.util.*;

/** This is the class for PGF grammars.*/
public class PGF {
	/** Reads a grammar in PGF format.
	 * @param path The path to the PGF file.
	 * @return A PGF object representing the grammar in memory. */
	public static native PGF readPGF(String path) throws FileNotFoundException;

	/** Reads a grammar from an input stream.
	 * @param stream The stream from which to read the grammar
	 * @return An object representing the grammar in memory. */
	// TODO: (?) public static native PGF readPGF(InputStream stream);

	/** Reads a grammar in PGF format and stores the unpacked data in a 
	 * platform-dependent NGF file, ready to be shared with other process, 
	 * or used for quick re-start.
	 * @param pgfPath The path to the PGF file.
	 * @param ngfPath The desired path for the new NGF file
	 * @return A PGF object representing the grammar in memory. */
	public static native PGF bootNGF(String pgfPath, String ngfPath) throws FileNotFoundException, FileAlreadyExistsException;

	/** Reads a grammar in NGF format. 
	 * @param path The path to the NGF file.
	 * @return A PGF object representing the grammar in memory.*/
	public static native PGF readNGF(String path) throws FileNotFoundException;

	/**
	 * Creates an empty grammar with the given abstract name and stores it
	 * on disk at the given path.
	 * @param abstractName The name of the empty grammar to create.
	 * @param path The path the grammar will be stored at.
	 * @return A PGF object representing the grammar in memory.
	 */
	public static native PGF newNGF(String abstractName, String path) throws FileAlreadyExistsException;

	/**
	 * Creates an empty grammar with the given abstract name, storing it only
	 * in memory.
	 * @param abstractName The name of the empty grammar to create.
	 * @return A PGF object representing the grammar in memory.
	 */
	public static PGF newNGF(String abstractName) throws FileAlreadyExistsException {
		return newNGF(abstractName, null);
	}

	/**
	 * Write pgf to the given path.
	 * @param path The path the grammar will be stored at.
	 */
	public native void writePGF(String path) throws FileAlreadyExistsException;

	/**
	 * Returns the name of the abstract syntax for the grammar.
	 * @return The name of the abstract syntax for the grammar.
	 */
	public native String getAbstractName();

	/**
	 * Returns a list of all categories in the grammar.
	 * @return A list of all categories in the grammar.
	 */
	public native List<String> getCategories();
	
	/**
	 * The given category's context.
	 * @param catname The name of a category.
	 * @return The list of `Hypo`s representing the category's context.
	 */
	public native List<Hypo> categoryContext(String catname);

	/**
	 * Returns the start category for the grammar.
	 * @return The start category for the grammar (as a `Type`, but we might
	 * want to change it back to a `String` since for instance `getCategories`
	 * returns a list of strings).
	 */
	public native Type getStartCat();

	/**
	 * Returns a list with the names of all functions in the grammar.
	 * @return A list with the names of all functions in the grammar.
	 */
	public native List<String> getFunctions();

	/** 
	 * Returns a list with all functions with a given return category.
	 * @param catname The name of the return category. 
	 * @return A list with all functions with a given return category.
	 */
	public native List<String> getFunctionsByCat(String catname);

	/**
	 * Checks whether or not the given function name is that of a constructor.
	 * @param funname The name of the function.
	 * @return true if the function is a constructor, false otherwise.
	 */
	public native boolean functionIsConstructor(String funname);

	/** Returns a map from a name of a concrete syntax to 
	 * a {@link Concr} object for the syntax. */
	//public native Map<String,Concr> getLanguages();
	
	
	/** Returns the type of the function with the given name.
	 * @param fun The name of the function.
	 * @return The type of the function with the given name.
	 */
	public native Type getFunctionType(String fun);

	/** Returns the negative logarithmic probability of the function
	 * with the given name.
	 * @param fun The name of the function.
	 * @return The negative logarithmic probability of the function
	 * with the given name.
	 */
	public native double getFunctionProbability(String fun);

	/** Returns an iterable over the set of all expression in
	 * the given category. The expressions are enumerated in decreasing
	 * probability order.
	 */
	//public Iterable<ExprProb> generateAll(String startCat) {
	//	return new Generator(this, startCat);
	//}

	/** Normalizes an expression to its normal form by using the 'def'
	 * rules in the grammar.
	 * 
	 * @param expr the original expression.
	 * @return the normalized expression.
	 */
	//public native Expr compute(Expr expr);

	/** Takes an expression and returns a refined version
	 * of the expression together with its type */
	//public native TypedExpr inferExpr(Expr expr) throws TypeError;
	
	/** Takes an expression and checks it agains a type. The returned expression
	 * is a possibly refined version of the original. */
	//public native Expr checkExpr(Expr expr, Type ty) throws TypeError;

	//public native String graphvizAbstractTree(Expr expr);

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
