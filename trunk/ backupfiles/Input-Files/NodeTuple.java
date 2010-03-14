import java.io.Serializable;

public class NodeTuple implements Serializable
{
	public int distance;
	public Node node;
	
	NodeTuple (int cDistance, Node cNode)
	{
		distance = cDistance;
		node = cNode;
	}
}
