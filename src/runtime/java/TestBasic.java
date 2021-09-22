import java.io.FileNotFoundException;

import org.junit.Test;
import static org.junit.Assert.*;

import org.grammaticalframework.pgf.*;

public class TestBasic {
  
  // readPGF

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