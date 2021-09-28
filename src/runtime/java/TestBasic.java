import java.io.File;
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

  // bootNGF

  @Test
  public void bootNGF_OK() {
    boolean thrown = false;
    String ngfPath = "new.ngf";
    try {
      PGF.bootNGF("../haskell/tests/basic.pgf", ngfPath);
    } catch (FileNotFoundException e) {
      thrown = true;
    }
    new File(ngfPath).delete();
    assertFalse(thrown);
  }

  @Test
  public void bootNGF_NotExisting() {
    boolean thrown = false;
    String ngfPath = "new.ngf";
    try {
      PGF.bootNGF("abc.pgf", ngfPath);
    } catch (IOException e) {
      thrown = true;
    }
  assertTrue(thrown);
  }

  @Test
  public void bootNGF_GF() throws FileNotFoundException {
    boolean thrown = false;
    String ngfPath = "new.ngf";
    try {
      PGF.bootNGF("../haskell/tests/basic.gf", ngfPath);
    } catch (PGFError e) {
      thrown = true;
    }
  assertTrue(thrown);
  }

  @Test
  public void bootNGF_NGF() throws FileNotFoundException {
    boolean thrown = false;
    String ngfPath = "new.ngf";
    try {
      PGF.bootNGF("basic.ngf", ngfPath);
    } catch (PGFError e) {
      thrown = true;
    }
  assertTrue(thrown);
  }

  @Test
  public void bootNGF_ExistingNgfPath() {
    boolean thrown = false;
    String ngfPath = "basic.ngf";
    try {
      PGF.bootNGF("../haskell/tests/basic.gf", ngfPath);
    } catch (IOException e) {
      thrown = true;
    }
    assertTrue(thrown);
  }

  // TODO: readNGF, newNGF

  // abstract syntax
  @Test
  public void getAbstractName_PGF() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals("basic", pgf.getAbstractName()); 
  }
}