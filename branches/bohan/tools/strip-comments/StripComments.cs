using System; using System.IO; using System.Text;

class StripComments {
	static void Main(String[] args) {
		if(args.Length == 0) args = new String[] {"."};
		foreach(String arg in args) StripDir(arg);
	}

	static void StripDir(String dir) {
		Console.WriteLine("walking " + dir);
		foreach(String pattern in new String[] {"*.js", "*.css"}) foreach(String file in Directory.GetFiles(dir, pattern)) StripFile(file);
		foreach(String subdir in Directory.GetDirectories(dir)) StripDir(subdir);
	}

	private static Encoding encoding = Encoding.Default; // or Encoding.UTF8

	private enum State { Normal, SingleLineComment, MultiLineComment, SingleQuotedString, DoubleQuotedString }

	static void StripFile(String filename) {
		Console.WriteLine("parsing " + filename);
		StringBuilder sb = new StringBuilder();
		using(StreamReader sr = new StreamReader(filename, encoding)) { String l; while((l = sr.ReadLine()) != null) sb.Append(l).Append('\n'); }
		String s = sb.ToString(); sb = new StringBuilder(); char prev = '\0', prevOutput = prev; State state = State.Normal, prevState = state;
		foreach(char c in s) {
			bool output = false; State newState = state;
			switch(state) {
				case State.SingleLineComment: if(c == '\n') { if(prevOutput != '\n') output = true; newState = State.Normal; } break;
				case State.MultiLineComment:                  if(prev == '*'  && c == '/' )         newState = State.Normal;   break;
				case State.SingleQuotedString: output = true; if(prev != '\\' && c == '\'')         newState = State.Normal;   break;
				case State.DoubleQuotedString: output = true; if(prev != '\\' && c == '"' )         newState = State.Normal;   break;
				case State.Normal: switch(prev) {
						case '\\': output = true; break;
						case '/': switch(c) {
								case '/': newState = State.SingleLineComment; break;
								case '*': newState = State.MultiLineComment;  break;
								default:
									if(prevState != State.MultiLineComment) { sb.Append('/'); output = true; }
									else switch(c) {
										case '\t': case ' ': case '\n': switch(prevOutput) {
												case '\t': case ' ': case '\n': break;
												default: output = true; break;
											} break;
										default: output = true; break;
									} break;
							} break;
						default: switch(c) {
								case '/': break;
								case '\'': output = true; newState = State.SingleQuotedString; break;
								case '"':  output = true; newState = State.DoubleQuotedString; break;
								case '\n': if(prevOutput != '\n') output = true; break;
								case '\t': case ' ': switch(prevOutput) {
										case '\t': case ' ': case '\n': break;
										default: output = true; break;
									} break;
								default: output = true; break;
							} break;
					} break;
			}
			prev = c; prevState = state; state = newState; if(output) { sb.Append(c); prevOutput = c; }
		}
		using(StreamWriter sw = new StreamWriter(filename, false, encoding)) { sw.Write(sb.ToString()); }
		//Console.Write(sb.ToString());
	}
}
