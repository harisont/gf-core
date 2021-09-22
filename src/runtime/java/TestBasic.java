import java.io.FileNotFoundException;

import org.junit.Test;
import static org.junit.Assert.*;

import org.grammaticalframework.pgf.*;

public class TestBasic {
  
  // readPGF

  @Test
  public void readPGFOnBasicPGFDoesNotThrowExceptions() {
    boolean thrown = false;
    try {
      PGF.readPGF("../haskell/tests/basic.pgf");
    } catch (Exception e) {
      thrown = true;
    }
  assertFalse(thrown);
  }
  
  @Test
  public void readPGFOnNonExistantThrowsFileNotFoundException() {
    boolean thrown = false;
    try {
      PGF.readPGF("nonexistant.pgf");
    } catch (FileNotFoundException e) {
      thrown = true;
    }
  assertTrue(thrown);
  }
}