public interface Searcher extends java.rmi.Remote
{
	public static final int DISTANCE_INFINITE = -1;
	public int getDistance (Node oFrom, Node oTo)throws java.rmi.RemoteException;
	public int getTransitiveDistance (int distance, Node oFrom, Node oTo)throws java.rmi.RemoteException;
}
