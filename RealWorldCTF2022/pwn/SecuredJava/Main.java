import java.util.*;
import java.lang.SecurityManager;
import java.io.*;
import com.h4ck1t.ctf.annotationprocessor.log.Log;

@Log
class Main {
    static {
        System.out.println("Hello");
    }

    public static void main(String[] args) throws IOException {
        SecurityManager security = System.getSecurityManager();
        System.out.println(security);
    }
}
