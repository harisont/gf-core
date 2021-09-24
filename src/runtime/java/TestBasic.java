import java.io.FileNotFoundException;
import java.io.IOException;

import org.junit.Test;
import static org.junit.Assert.*;

import org.grammaticalframework.pgf.*;

public class TestBasic {
  
  // readPGF

  @Test
  public void readPGF_PGF() {
    boolean thrown = false;
    try {
      PGF.readPGF("../haskell/tests/basic.pgf");
    } catch (Exception e) {
      thrown = true;
    }
  assertFalse(thrown);
  }
  
  @Test
  public void readPGF_NonExistant() {
    boolean thrown = false;
    try {
      PGF.readPGF("abc.pgf");
    } catch (IOException e) {
      thrown = true;
    }
  assertTrue(thrown);
  }

  @Test
  public void readPGF_GF() throws FileNotFoundException {
    boolean thrown = false;
    try {
      PGF.readPGF("../haskell/tests/basic.gf");
    } catch (PGFError e) {
      thrown = true;
    }
    assertTrue(thrown);
  }

  @Test
  public void readPGF_NGF() throws FileNotFoundException {
    boolean thrown = false;
    try {
      PGF.readPGF("basic.ngf");
    } catch (PGFError e) {
      thrown = true;
    }
    assertTrue(thrown);
  }

  // TODO: bootNGF, readNGF, newNGF

  // abstract syntax
  @Test
  public void getAbstractName_PGF() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals("basic", pgf.getAbstractName()); 
  }
}