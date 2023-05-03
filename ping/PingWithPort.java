import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class PingWithPort
{
	static ArrayList<String> arg=new ArrayList<>();
	static Map<String, Object> argument=new HashMap<>();
	static Map<String, Object> defaults=new HashMap<>();

	public static void main(String[] args)
	{
		for(String e:args)
			arg.add(e);
		if(arg.size()==0)
			displayHelp();
		else
		{
			argument.put("target", arg.get(0));
			for(Option opt:Option.values())
				getValuesForOption(opt);
			int port=(int) argument.get("port");
			int count=(int) argument.get("count");
			int timeout=(int) argument.get("timeout");
			ArrayList<Long> results=new ArrayList<>();
			int success=0;
			int fail=0;
			InetSocketAddress soc=new InetSocketAddress((String)argument.get("target"), port);
			System.out.println("Pinging "+argument.get("target")+(port==0? "":":"+argument.get("port"))+" ["+soc.getAddress().getHostAddress()+"]:");
			for(int i=0; i<count; i++)
			{	
				long result=-1;
				if(port!=0)
					result=pingHost(soc, timeout);
				else
					result=pintHost(soc.getAddress(), timeout);
				if(result==-1)
					fail++;
				else
				{
					results.add(result);
					success++;
				}
			}
			long min=timeout, max=-1;
			long sum=0;
			Iterator<Long> i=results.iterator();
			while(i.hasNext())
			{
				long cur=i.next();
				min=cur<min? cur:min;
				max=cur>max? cur:max;
				sum+=cur;
			}
			System.out.println("Ping statsics for "+soc.getAddress().getHostAddress()+":");
			System.out.println("\tPackets: Sent = "+count+", Received = "+success+", Lost = "+fail+" ("+fail/count*100+"% loss),");
			if(success>0)
			{
				System.out.println("Approximate round trip times in milli-seconds:");
				System.out.println("\tMinimum = "+min+"ms, Maximum = "+max+"ms, Average = "+sum/success+"ms");
			}
		}
		
	}
	private static void displayHelp()
	{
		System.out.println("Usage:\ttarget [-p port] [-w timeout] [-n count]\n");
		System.out.println("Options:");
		for(Option opt:Option.values())
		{
			System.out.printf("\t%-16s%s\n", opt.help.key, opt.help.desc);
		}
		
	}
	public static void getValuesForOption(Option opt)
	{
		if(arg.contains(opt.key))
		{
			String tmp=null;
			try
			{
				if(opt!=null && opt.needValue)
				{
					tmp=arg.get(arg.indexOf(opt.key)+1);
					if(opt.type==Type.BOOLEAN)
					{
						if(tmp=="true" || tmp=="false")	
							argument.put(opt.name, Boolean.parseBoolean(tmp));
						else
							throw new IllegalArgumentException("boolean");
					}
					else if(opt.type==Type.NUMBER)
					{
						try
						{
							int tmp2=Integer.parseInt(tmp);
							argument.put(opt.name, tmp2);
						}
						catch(NumberFormatException e)
						{
							throw new IllegalArgumentException("integer");
						}
					}
					else if(opt.type==Type.DOUBLE)
					{
						try
						{
							double tmp2=Double.parseDouble(tmp);
							argument.put(opt.name, tmp2);
						}
						catch(NumberFormatException e)
						{
							throw new IllegalArgumentException("double");
						}
					}
					else if(opt.type==Type.STRING)
						argument.put(opt.name, tmp);
					
				}
			}
			catch(IndexOutOfBoundsException e)
			{
				System.out.println("Unable to get value for option "+opt.key+".");
			}
			catch(IllegalArgumentException e)
			{
				System.out.println("Invalid value for option "+opt.key+". Needed type: \""+e.getMessage()+"\".");
			}

		}
		else
			loadDefaultsForOption(opt);
	}
	private static void loadDefaultsForOption(Option opt)
	{
		defaults.put("timeout", 10000);
		defaults.put("count", 4);
		defaults.put("port", 0);
		
		argument.put(opt.name, defaults.get(opt.name));
	}
	public static long pintHost(InetAddress target, int timeout)
	{
		try
		{
			long startTime=System.currentTimeMillis();
			target.isReachable(timeout);
			long endTime=System.currentTimeMillis();
			long duration=endTime-startTime;
			System.out.println("Reply from "+target.getHostAddress()+": time"+(duration==0? "<":"=")+duration+"ms");
			return duration;
		}
		catch (IOException e)
		{
			System.out.println("Request timed out.");
			return -1;
		}
		
	}
	public static long pingHost(InetSocketAddress soc, int timeout)
	{
		try(Socket socket=new Socket())
		{
			long startTime=System.currentTimeMillis();
			socket.connect(soc, timeout);
			long endTime=System.currentTimeMillis();
			long duration=endTime-startTime;
			System.out.println("Reply from "+soc.getAddress().getHostAddress()+": time"+(duration==0? "<":"=")+duration+"ms");
			return duration;
		}
		catch (IOException e)
		{
			System.out.println("Request timed out.");
			return -1;
		}
	}
	public enum Option
	{
		
		TIMEOUT("timeout", "-w", true, Type.NUMBER, Help.TIMEOUT),
		COUNT("count", "-n", true, Type.NUMBER, Help.COUNT),
		PORT("port", "-p", true, Type.NUMBER, Help.PORT);
		
		String name;
		String key;
		boolean needValue;
		Type type;
		Help help;
		Option(String name, String key, boolean needValue, Type type, Help help)
		{
			this.name=name;
			this.key=key;
			this.needValue=needValue;
			this.type=type;
			this.help=help;
		}
		Option(String name, String key, Help help)
		{
			this(name, key, false, Type.NONE, help);
		}
	}
	public enum Type
	{
		NUMBER(),
		DOUBLE(),
		STRING(),
		BOOLEAN(),
		NONE();
	}
	public enum Help
	{
		TIMEOUT("-w timeout", "Timeout in milliseconds to wait for each reply."),
		COUNT("-n count", "Number of echo requests to send."),
		PORT("-p port", "Ping a specific port on the host.");
		
		String key;
		String desc;
		
		Help(String key, String desc)
		{
			this.key=key;
			this.desc=desc;
		}
	}
}
