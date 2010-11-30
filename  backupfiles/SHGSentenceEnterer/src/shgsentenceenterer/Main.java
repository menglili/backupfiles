/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package shgsentenceenterer;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.lang.*;
import java.util.List;
import java.util.Scanner;
import java.util.Vector;

/**
 *
 * @author Fin
 */

class Token
{
    public Integer position;
    public String token;
    Character type;
    String flags = "";
}

class Sentence
{
    public Integer tokenCount = 0;
    public Integer wordCount = 0;
    public String language = "en";
    public List<Token> tokens = new Vector<Token>();
    
    public void clear()
    {
        tokenCount = 0;
        wordCount = 0;
        language = "en";
        tokens.clear();
    }
}



class Enterer
{
    PrintWriter writer = null;
    
    public Enterer() throws IOException
    {
    }
      
   public void run(String filename, String batchName) throws FileNotFoundException,IOException
    {
       writer = new PrintWriter(new FileWriter(new File(filename + ".sql")));
       Scanner reader = new Scanner(new BufferedReader(new FileReader(new File(filename))));
       String line = "";
       Sentence sentence = new Sentence();

       while(reader.hasNextLine())
       {
           line = reader.nextLine();
           if(sentence.tokenCount > 0 && line.charAt(0) == '<' && ( line.charAt(1) == 's' || 
                   (line.charAt(1) == '/' && line.charAt(2) == 'c')))
           {
               generateSentence(sentence, batchName);
               sentence.clear();
           }
           else
            parse(line,sentence);
       }
       
       reader.close();
       writer.flush();
       writer.close();
    }
   
   
   void parse(String line, Sentence sentence)
   {
        int i = 0;
        while(line.charAt(i) == ' ') ++i;
        line = line.substring(i);
        if(line.charAt(0) != '<') return;
        if(line.charAt(1)!='f' && line.charAt(1)!='d') return;
        if(line.charAt(2) != ' ') return;
        
        sentence.tokenCount++;
        
        Token adding = new Token();
        
        if(line.charAt(1) == 'f')
        {
            adding.type = 'W';
            sentence.wordCount++;
        }else adding.type = 'P';
       
        adding.position = (sentence.tokens.isEmpty() ? 1 : sentence.tokens.get(sentence.tokens.size()-1).position + 1);
        
        i = 0;
        while(line.charAt(i)!= '>')++i;
        ++i;
        int j = i;
        while(line.charAt(j) != '<')
        {
            ++j;
        }
        String rawToken = line.substring(i, j);
        adding.token = "";
        for(int k=0;k<rawToken.length();++k){
            if(rawToken.charAt(k) == '\''){
                adding.token += "''";
            }else{
                adding.token += rawToken.charAt(k);
            }
        }
        
        int tagPos = line.indexOf("<t>");
        tagPos+= 3;
        String tag = "";
        while(line.charAt(tagPos) != '<'){
            tag +=line.charAt(tagPos++);
        }

        adding.flags = tag;
        
        sentence.tokens.add(adding);
        return;
   }
   
   void generateSentence(Sentence sentence, String batchName)
   {
       writer.println("INSERT INTO sentences ( tokens, words, language, batch ) VALUES ( "
               + sentence.tokenCount + ", " + sentence.wordCount + ", " + wrap(sentence.language)
               + ", " + wrap(batchName) + " );");
       
       for(Token t : sentence.tokens)
       {
          writer.println("INSERT INTO tokens ( sentence_id, position, token, type, flags ) VALUES ( " + 
                  "(SELECT MAX(id) FROM sentences), " + t.position + ", " + wrap(t.token) + ", " + 
                  wrap(t.type) + ", " + wrap(t.flags) + " );");
       }
    }

   private String wrap(String word)
   {
        return ("'" + word + "'");
   }

    private String wrap(Character chr) {
        return ("'" + chr + "'");
    }
}
public class Main {

    /**
     * @param args the command line arguments
     */
     
    public static void main(String[] args) throws FileNotFoundException, IOException {

        if(args.length < 1){
            System.out.println("Usage: SGHSentenceEnterer input file [batch name]");
            return;
        }

        Enterer enterer = new Enterer();        
        if(args.length < 2)
            enterer.run(args[0], "");
        else
            enterer.run(args[0], args[1]);
    }

}
