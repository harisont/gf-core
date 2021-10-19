import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.file.FileAlreadyExistsException;
import java.util.Arrays;
import java.util.List;

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
  
  /* PGF */
  
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

  @Test
  public void readNGF_BasicHasCategories() throws FileNotFoundException, FileAlreadyExistsException {
    createBasicNGF();
    PGF pgf = PGF.readNGF("basic.ngf");
    assertTrue(pgf.getCategories().size() > 0);
  }

  // newNGF
  
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

  // writePGF

  @Test
  public void writePGF_OK() throws FileNotFoundException {
    boolean thrown = false;
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    String path = "copy.pgf";
    try {
      pgf.writePGF(path);
    } catch (Exception e) {
      thrown = true;
    }
    new File(path).delete();
    assertFalse(thrown);
  }

  // getAbstractName

  @Test
  public void getAbstractName_OK() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals("basic", pgf.getAbstractName()); 
  }

  // getCategories

  @Test
  public void getCategories_OK() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    String[] catsArray = {"Float","Int","N","P","S","String"};
    assertEquals(Arrays.asList(catsArray), pgf.getCategories());
  }

  // categoryContext

  @Test
  public void categoryContext_N() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    Hypo[] ctx = {};
    assertEquals(Arrays.asList(ctx), pgf.categoryContext("N"));
  }

  @Test
  public void categoryContext_S() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    Hypo[] ctx = {};
    assertEquals(Arrays.asList(ctx), pgf.categoryContext("S"));
  }

  @Test
  public void categoryContext_P() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    List<Hypo> ctx = pgf.categoryContext("P");
    assertEquals(1, ctx.size());
    Hypo h = ctx.get(0);
    assertEquals(true, h.getBindType());
    assertEquals("_", h.getVar());
    assertEquals(Type.readType("N"), h.getType());
  }

  // getStartCat

  @Test
  public void getStartCat_OK() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals(Type.readType("S"), pgf.getStartCat());
  }

  // getFunctions

  @Test
  public void getFunctions_OK() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    String[] funsArray = {"c","ind","s","z"};
    assertEquals(Arrays.asList(funsArray), pgf.getFunctions());
  }

  // getFunctionsByCat

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

  // functionIsConstructor

  @Test
  public void functionIsConstructor_s() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals(true, pgf.functionIsConstructor("s"));
  }

  @Test
  public void functionIsConstructor_z() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals(true, pgf.functionIsConstructor("z"));
  }

  @Test
  public void functionIsConstructor_c() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals(true, pgf.functionIsConstructor("c"));
  }

  @Test
  public void functionIsConstructor_ind() throws FileNotFoundException {
    PGF pgf = PGF.readPGF("../haskell/tests/basic.pgf");
    assertEquals(false, pgf.functionIsConstructor("ind"));
  }

  /* Type */

  // readType

  @Test 
  public void readType_OK() throws FileNotFoundException {
    boolean thrown = false;
    try {
      Type.readType("S");
    } catch (PGFError e) {
      thrown = true;
    }
    assertFalse(thrown);
  }

  @Test
  public void readType_InvalidType() throws FileNotFoundException {
    boolean thrown = false;
    try {
      Type.readType("->");
    } catch (PGFError e) {
      thrown = true;
    }
    assertTrue(thrown);
  }

  @Test
  public void readType_ExactEquality() {
    assertEquals(Type.readType("A"), Type.readType("A"));
  }

  @Test
  public void readType_EqualityWithWhitespace() {
    assertEquals(Type.readType("A -> B"), Type.readType("A->B"));
  }

  @Test
  public void readType_EqualityWithMoreWhitespace() {
    assertEquals(Type.readType("A -> B -> C"), Type.readType("A->B   ->   C"));
  }

  @Test
  public void readType_Inequality() {
    assertNotEquals(Type.readType("A"), Type.readType("B"));
  }

  @Test
  public void readType_InequalityWithWhitespace() {
    assertNotEquals(Type.readType("A -> B"), Type.readType("B->B"));
  }
}