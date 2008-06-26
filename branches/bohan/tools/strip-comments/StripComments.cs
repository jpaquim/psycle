using System; using System.IO; using System.Text;

class StripComments {
	static void Main(String[] args) {
		if(args.Length == 0) args = new String[] {"."};
		foreach(String arg in args) StripComments.StripDir(arg);
	}

	static void StripDir(String dir) {
		Console.WriteLine("walking " + dir);
		foreach(String pattern in new String[] {"*.js", "*.css"}) foreach(String file in Directory.GetFiles(dir, pattern)) StripFile(file);
		foreach(String subdir in Directory.GetDirectories(dir)) StripDir(subdir);
	}

	private enum State { Normal, SingleLineComment, MultiLineComment, SingleQuotedString, DoubleQuotedString }

	static void StripFile(String filename) {
		Console.WriteLine("parsing " + filename);
		StringBuilder sb = new StringBuilder();
		//String s = File.ReadAllText(filename);
		using(StreamReader sr = new StreamReader(filename)) { String l; while((l = sr.ReadLine()) != null) sb.Append(l).Append('\n'); }
		String s = sb.ToString();
		sb = new StringBuilder(); char prev = '\0'; State state = State.Normal, prevState = state;
		foreach(char c in s) {
			State newState = state;
			switch(state) {
				case State.SingleLineComment:  if(                c == '\n') newState = State.Normal; break;
				case State.MultiLineComment:   if(prev == '*'  && c == '/' ) newState = State.Normal; break;
				case State.SingleQuotedString: if(prev != '\\' && c == '\'') newState = State.Normal; sb.Append(c); break;
				case State.DoubleQuotedString: if(prev != '\\' && c == '"' ) newState = State.Normal; sb.Append(c); break;
				case State.Normal: switch(prev) {
						case '\\': sb.Append(c); break;
						case '/': switch(c) {
								case '/': newState = State.SingleLineComment; break;
								case '*': newState = State.MultiLineComment;  break;
								default: if(prevState != State.MultiLineComment) sb.Append(prev); sb.Append(c); break;
							} break;
						default: switch(c) {
								case '\'': newState = State.SingleQuotedString; sb.Append(c); break;
								case '"':  newState = State.DoubleQuotedString; sb.Append(c); break;
								case '/': break;
								default: sb.Append(c); break;
							}
							break;
					} break;
			}
			prev = c; prevState = state; state = newState;
		}
		Console.Write(sb.ToString());
	}
}
