import java.io.FileNotFoundException;

import org.junit.Test;
import static org.junit.Assert.assertTrue;

import org.grammaticalframework.pgf.*;

public class TestBasic {
  
  @Test
  public void readPGFNonExistantThrowsFileNotFoundException() {
    boolean thrown = false;
    try {
      PGF.readPGF("nonexistant.pgf");
    } catch (FileNotFoundException e) {
      thrown = true;
    }
  assertTrue(thrown);
  }
}