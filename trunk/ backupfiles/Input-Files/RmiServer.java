import java.rmi.Naming;
import java.rmi.RMISecurityManager;

public class RmiServer
{	
	public static void main (String args [])
	{
		// Create and install a security manager if necessary.
		if (System.getSecurityManager () == null)
			System.setSecurityManager (new RMISecurityManager ());

		try
		{ 
			// Instantiate the remotely accessible object. The constructor
			// of the object automatically exports it for remote invocation.
			SearcherImpl searcher = new SearcherImpl ();
			NodeCreatorImpl nodeCreator = new NodeCreatorImpl();
			// Use the registry on this host to register the server.
			// The host name must be changed if the server uses
			// another computer than the client!
			Naming.rebind ("//localhost/Searcher", searcher);
			Naming.rebind ("//localhost/NodeCreator", nodeCreator);
			// The virtual machine will not exit here because the export of
			// the remotely accessible object creates a new thread that
			// keeps the application active.
		}
		catch (Exception e)
		{
			System.out.println ("Server Exception: " + e.getMessage ());
			e.printStackTrace (); 
		}
	} 
}
