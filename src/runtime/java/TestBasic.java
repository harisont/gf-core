import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.file.FileAlreadyExistsException;
import java.util.Arrays;

import org.junit.Test;
import static org.junit.Assert.*;

import org.grammaticalframework.pgf.*;

public class TestBasic {

  private void createBasicNGF() throws FileAlreadyExistsException, FileNotFoundException {
      String ngfPath = "basic.ngf";

        // if basic.ngf already exists, remove it, just in case
        File ngfFile = new File(ngfPath);
        if (ngfFile.exists())
            ngfFile.delete();
        
        // create basic.ngf  
        PGF.bootNGF("../haskell/tests/basic.pgf", ngfPath);
  }
  
  // readPGF

  @Test
  public void readPGF_OK() {
    boolean thrown = false;
    try {
      PGF.readPGF("../haskell/tests/basic.pgf");
    } catch (Exception e) {
      thrown = true;
    }
  assertFalse(thrown);
  }
  
  @Test
  public void readPGF_NotExisting() {
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
  public void readPGF_NGF() throws FileNotFoundException, FileAlreadyExistsException {
    createBasicNGF();
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
    } catch (IOException e) {
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
  public void bootNGF_GF() throws FileNotFoundException, FileAlreadyExistsException {
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
  public void bootNGF_NGF() throws FileNotFoundException, FileAlreadyExistsException {
    createBasicNGF();
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

  // readNGF

  @Test
  public void readNGF_OK() throws FileAlreadyExistsException, FileNotFoundException {
    createBasicNGF();
    boolean thrown = false;
    try {
      PGF.readNGF("basic.ngf");
    } catch (Exception e) {
      thrown = true;
    }
  assertFalse(thrown);
  }
  
  @Test
  public void readNGF_NotExisting() {
    boolean thrown = false;
    try {
      PGF.readNGF("abc.ngf");
    } catch (IOException e) {
      thrown = true;
    }
  assertTrue(thrown);
  }

  @Test
  public void readNGF_GF() throws FileNotFoundException {
    boolean thrown = false;
    try {
      PGF.readNGF("../haskell/tests/basic.gf");
    } catch (PGFError e) {
      thrown = true;
    }
    assertTrue(thrown);
  }

  @Test
  public void readNGF_PGF() throws FileNotFoundException {
    boolean thrown = false;
    try {
      PGF.readNGF("../haskell/tests/basic.pgf");
    } catch (PGFError e) {
      thrown = true;
    }
    assertTrue(thrown);
  }

  // TODO: something like Python's test_readNGF
  
  @Test
  public void newNGF_File() throws FileAlreadyExistsException {
    String path = "empty.ngf";
    PGF pgf = PGF.newNGF("empty", path);
    assertEquals(0, pgf.getCategories().size());
    new File(path).delete();
  }

  @Test
  public void newNGF_Memory() {
    PGF pgf = PGF.newNGF("empty");
    assertEquals(0, pgf.getCategories().size());
  }

  @Test
  public void newNGF_ExixtingFile() throws FileAlreadyExistsException, FileNotFoundException {
    createBasicNGF();
    boolean thrown = false;
    try {
      PGF.newNGF("basic","basic.ngf");
    } catch (IOException e) {
      thrown = true;
    }
    assertTrue(thrown);
  }

  // abstract syntax

  @Test
  public void getAbstractName_OK() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals("basic", pgf.getAbstractName()); 
  }

  @Test
  public void getCategories_OK() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    String[] catsArray = {"Float","Int","N","P","S","String"};
    assertEquals(Arrays.asList(catsArray), pgf.getCategories());
  }

  @Test
  public void getFunctions_OK() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    String[] funsArray = {"c","ind","s","z"};
    assertEquals(Arrays.asList(funsArray), pgf.getFunctions());
  }

  @Test
  public void getFunctionsByCat_N() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    String[] funsArray = {"s","z"};
    assertEquals(Arrays.asList(funsArray), pgf.getFunctionsByCat("N"));
  }

  @Test
  public void getFunctionsByCat_S() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    String[] funsArray = {"c"};
    assertEquals(Arrays.asList(funsArray), pgf.getFunctionsByCat("S"));
  }

  @Test
  public void getFunctionsByCat_X() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    String[] funsArray = {};
    assertEquals(Arrays.asList(funsArray), pgf.getFunctionsByCat("X"));
  }

  // types

  @Test
  public void readType_Invalid() throws FileNotFoundException {
    boolean thrown = false;
    try {
      Type.readType("->");
    } catch (PGFError e) {
      thrown = true;
    }
    assertTrue(thrown);
  }

  @Test // TODO: remove, it isn't really relevant
  public void readType_Valid() throws FileNotFoundException {
    boolean thrown = false;
    try {
      Type.readType("S");
    } catch (PGFError e) {
      thrown = true;
    }
    assertFalse(thrown);
  }
}