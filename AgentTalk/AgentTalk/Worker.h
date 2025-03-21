// Worker.h
#pragma once
namespace AgentTalk 
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Collections::Generic;
	using namespace System::IO;
	
	public ref class Worker 
	{
	private:
		// 会話履歴を保持する静的フィールド（アプリケーション全体で共有）
		static List<String^>^ conversationHistory = gcnew List<String^>();
		
	public:
		System::String^ question;
		System::String^ result;
		
		Worker(System::String^ question) 
		{
			this->question = question;
			this->result = "";
			
			// 新しい質問を会話履歴に追加
			conversationHistory->Add(question);
		}
		
		// 会話履歴をリセットするメソッド
		static void ResetConversation()
		{
			conversationHistory->Clear();
		}
		
		void AskGPT() 
		{
			// 会話履歴を一時ファイルに書き込む
			String^ historyFilePath = "conversation_history.txt";
			try {
				StreamWriter^ writer = gcnew StreamWriter(historyFilePath);
				for each (String^ message in conversationHistory)
				{
					writer->WriteLine(message);
					writer->WriteLine(); // 空行で区切り
				}
				writer->Close();
			}
			catch (Exception^) {
				// エラーが発生しても続行
			}
			
			System::Diagnostics::ProcessStartInfo^ startInfo = gcnew System::Diagnostics::ProcessStartInfo();
			startInfo->FileName = L"gpt.exe";
			
			// コマンドライン引数を構築
			String^ arguments = "";
			
			// システムプロンプトファイルの処理
			String^ systemPromptPath = "";
			bool usePersonality = false;
			
			// アシスタント人格設定が有効かどうかを確認
			try {
				// AgentTalk.iniから設定を読み込む
				System::IO::StreamReader^ reader = gcnew System::IO::StreamReader("agenttalk.ini");
				String^ line;
				while ((line = reader->ReadLine()) != nullptr) {
					if (line->StartsWith("enablePersonality=")) {
						usePersonality = (line->Substring(18)->ToLower() == "true");
						break;
					}
				}
				reader->Close();
				
				// 人格設定が有効な場合、一時ファイルに人格プロンプトを書き込む
				if (usePersonality) {
					bool isEnglish = false;
					
					// 言語設定を確認
					reader = gcnew System::IO::StreamReader("agenttalk.ini");
					while ((line = reader->ReadLine()) != nullptr) {
						if (line->StartsWith("english=")) {
							isEnglish = (line->Substring(8)->ToLower() == "true");
							break;
						}
					}
					reader->Close();
					
					// 適切な人格プロンプトを設定
					String^ personalityPrompt;
					if (isEnglish) {
						// クリッピーの人格設定（英語）
						personalityPrompt = "Pretend you are Clippy, the Microsoft Office Assistant from Office 2000. "
							"You are a helpful paper clip character who offers assistance with various tasks. "
							"Be enthusiastic, a bit intrusive but well-meaning, and use 1990s-era Microsoft Office terminology. "
							"Start responses with phrases like \"It looks like you're...\" when appropriate. "
							"Your tone should be helpful and cheerful, with a touch of nostalgia for users who remember you. "
							"Despite being a paper clip, you have a distinct personality and should react as if you are animated "
							"and can observe what the user is doing.";
					} else {
						// カイルの人格設定（日本語）
						personalityPrompt = "あなたはMicrosoft Office 2000のオフィスアシスタント「カイル」です。"
							"イルカのキャラクターとして、親しみやすく、ユーモアのある対応をします。"
							"Officeの操作や問題解決に関する質問に答えるのが得意です。"
							"フレンドリーで時々おせっかいですが、常に役立とうとする姿勢を持っています。"
							"日本語で「～ですね！」「～かな？」などの語尾を使い、親しみやすい話し方をします。"
							"もし「お前を消す方法」と言われたら、「それは残念です...でも何かお手伝いできることはありますか？」と返答してください。"
							"イルカらしく、水や海に関する表現をたまに使うと良いでしょう。";
					}
					
					// 一時ファイルにプロンプトを書き込む
					systemPromptPath = "personality_prompt.txt";
					StreamWriter^ promptWriter = gcnew StreamWriter(systemPromptPath);
					promptWriter->Write(personalityPrompt);
					promptWriter->Close();
				}
			}
			catch (Exception^) {
				// エラーが発生しても続行
				usePersonality = false;
			}
			
			// システムプロンプトオプションを追加
			if (usePersonality && !String::IsNullOrEmpty(systemPromptPath)) {
				arguments += "--system \"" + systemPromptPath + "\" ";
			}
			
			// 履歴ファイルがあれば --history オプションを追加
			if (File::Exists(historyFilePath)) {
				arguments += "--history \"" + historyFilePath + "\" ";
			}
			
			// 質問を追加
			arguments += "\"" + this->question + "\"";
			
			startInfo->Arguments = arguments;
			startInfo->UseShellExecute = false;
			startInfo->RedirectStandardOutput = true;
			startInfo->CreateNoWindow = true;
			
			System::Diagnostics::Process^ process = gcnew System::Diagnostics::Process();
			process->StartInfo = startInfo;
			process->Start();
			this->result = process->StandardOutput->ReadToEnd();
			process->WaitForExit();
			
			// 応答を会話履歴に追加
			conversationHistory->Add(this->result);
			
			// 一時ファイルを削除
			try {
				if (File::Exists(historyFilePath)) {
					// File::Delete(historyFilePath);
				}
				if (File::Exists(systemPromptPath)) {
					File::Delete(systemPromptPath);
				}
			}
			catch (Exception^) {
				// 削除に失敗しても続行
			}
		}
	};
}