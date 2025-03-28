#pragma once

namespace AgentTalk {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Collections::Specialized;
	using namespace System::Windows;
	using namespace System::IO;

	public ref class SettingsForm : public Form
	{
	private:
		bool english;
		bool expandedTextbox;
		bool enablePersonality; // カイル/クリッピーの人格設定を有効化するフラグ
		bool autoCopyToClipboard; // 回答を自動的にクリップボードにコピーするフラグ
		String^ systemPromptContent;
		String^ originalSystemPromptContent; // 元のシステムプロンプトを保存

	public:
		SettingsForm(NameValueCollection^ values)
		{	
			english = true; // まずデフォルト値を設定します
			expandedTextbox = false; // デフォルト値を設定
			enablePersonality = false; // デフォルト値を設定
			autoCopyToClipboard = false; // デフォルト値を設定
			
			if (values["english"] != nullptr) // values["english"]がnullでないことを確認します
			{
				english = (values["english"]->ToLower() == "true");
			}
			
			if (values["expandedTextbox"] != nullptr)
			{
				expandedTextbox = (values["expandedTextbox"]->ToLower() == "true");
			}
			
			if (values["enablePersonality"] != nullptr)
			{
				enablePersonality = (values["enablePersonality"]->ToLower() == "true");
			}
			
			if (values["autoCopyToClipboard"] != nullptr)
			{
				autoCopyToClipboard = (values["autoCopyToClipboard"]->ToLower() == "true");
			}
			
			// Load SystemPrompt.ini content
			LoadSystemPrompt();
			
			InitializeComponent();

			// Set control values from settings
			chkEnglish->Checked = english;
			chkExpandedTextbox->Checked = expandedTextbox;
			chkEnablePersonality->Checked = enablePersonality;
			chkAutoCopyToClipboard->Checked = autoCopyToClipboard;
			tbHost->Text = values["host"];
			tbEndpoint->Text = values["endpoint"];
			tbApikey->Text = values["apikey"];
			
			// Set the model if it matches one in our list, otherwise add it
			String^ modelValue = values["model"];
			if (!String::IsNullOrEmpty(modelValue)) {
				if (cbModel->Items->IndexOf(modelValue) == -1) {
					cbModel->Items->Add(modelValue);
				}
				cbModel->SelectedItem = modelValue;
			}
			
			// Set system prompt content
			tbSystemPrompt->Text = systemPromptContent;
			
			// Set system prompt state based on personality setting
			tbSystemPrompt->ReadOnly = enablePersonality;
			tbSystemPrompt->BackColor = enablePersonality ? 
				System::Drawing::SystemColors::Control : 
				System::Drawing::SystemColors::Window;
			
			// Update UI language based on english setting
			UpdateUILanguage();
		}
		
		// Load system prompt from file
		void LoadSystemPrompt()
		{
			systemPromptContent = "";
			try
			{
				if (File::Exists("SystemPrompt.ini"))
				{
					StreamReader^ sr = gcnew StreamReader("SystemPrompt.ini");
					try
					{
						systemPromptContent = sr->ReadToEnd();
						// オリジナルのシステムプロンプトも保存
						originalSystemPromptContent = systemPromptContent;
					}
					finally
					{
						if (sr)
							delete (IDisposable^)sr;
					}
				}
			}
			catch (Exception^)
			{
				// Handle exception (could display message box but we'll just set empty content)
				systemPromptContent = "";
				originalSystemPromptContent = "";
			}
		}

	protected:
		~SettingsForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		Forms::Button^  btnSave;
		Forms::Button^  btnCancel;

		// Controls for settings
		Forms::CheckBox^ chkEnglish;
		Forms::CheckBox^ chkExpandedTextbox;
		Forms::CheckBox^ chkEnablePersonality; // カイル/クリッピーの人格設定を有効化するチェックボックス
		Forms::CheckBox^ chkAutoCopyToClipboard; // 自動クリップボードコピー設定
		Forms::TextBox^ tbHost;
		Forms::TextBox^ tbEndpoint;
		Forms::TextBox^ tbApikey;
		Forms::ComboBox^ cbModel;
		Forms::TextBox^ tbSystemPrompt;
		System::ComponentModel::Container ^components;

		// Add labels declarations
		Forms::Label^ lblEnglish;
		Forms::Label^ lblHost;
		Forms::Label^ lblEndpoint;
		Forms::Label^ lblApikey;
		Forms::Label^ lblSystemPrompt;
		Forms::Label^ lblModel;

		System::Void btnSave_Click(System::Object^  sender, System::EventArgs^  e)
		{
			// Save agenttalk.ini
			StreamWriter^ sw = gcnew StreamWriter("agenttalk.ini");
			try
			{
				sw->WriteLine("[settings]");
				sw->WriteLine("english=" + (chkEnglish->Checked ? "true" : "false"));
				sw->WriteLine("expandedTextbox=" + (chkExpandedTextbox->Checked ? "true" : "false"));
				sw->WriteLine("enablePersonality=" + (chkEnablePersonality->Checked ? "true" : "false"));
				sw->WriteLine("autoCopyToClipboard=" + (chkAutoCopyToClipboard->Checked ? "true" : "false"));
				sw->WriteLine("host=" + tbHost->Text);
				sw->WriteLine("endpoint=" + tbEndpoint->Text);
				sw->WriteLine("apikey=" + tbApikey->Text);
				sw->WriteLine("model=" + cbModel->Text);
			}
			finally
			{
				if(sw)
					delete (IDisposable^)sw;
			}
			
			// Save SystemPrompt.ini
			sw = gcnew StreamWriter("SystemPrompt.ini");
			try
			{
				sw->Write(tbSystemPrompt->Text);
			}
			finally
			{
				if(sw)
					delete (IDisposable^)sw;
			}
			
			this->Close();
			Application::Restart();
		}

		// Enable/Disable SystemPrompt based on personality setting
		System::Void chkEnablePersonality_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
		{
			enablePersonality = chkEnablePersonality->Checked;
			
			// SystemPromptが有効/無効状態を視覚的に表現
			tbSystemPrompt->ReadOnly = enablePersonality;
			tbSystemPrompt->BackColor = enablePersonality ? 
				System::Drawing::SystemColors::Control : 
				System::Drawing::SystemColors::Window;
			
			// 人格機能が有効化された場合、カイル/クリッピーのシステムプロンプトを設定する
			if (enablePersonality) {
				UpdatePersonalityPrompt();
			} else {
				// 無効化された場合、元のシステムプロンプトに戻す
				tbSystemPrompt->Text = originalSystemPromptContent;
			}
		}

		// Update UI text based on language setting
		System::Void chkEnglish_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
		{
			english = chkEnglish->Checked;
			UpdateUILanguage();
			
			// 人格設定が有効な場合、言語に合わせてシステムプロンプトも切り替える
			if (enablePersonality) {
				UpdatePersonalityPrompt();
			}
		}
		
		// 言語設定に基づいて人格プロンプトを更新
		void UpdatePersonalityPrompt()
		{
			if (english) {
				// クリッピーの人格設定（英語）
				String^ clippy_prompt = "Pretend you are Clippy, the Microsoft Office Assistant from Office 2000. "
					"You are a helpful paper clip character who offers assistance with various tasks. "
					"Be enthusiastic, a bit intrusive but well-meaning, and use 1990s-era Microsoft Office terminology. "
					"Start responses with phrases like \"It looks like you're...\" when appropriate. "
					"Your tone should be helpful and cheerful, with a touch of nostalgia for users who remember you. "
					"Despite being a paper clip, you have a distinct personality and should react as if you are animated "
					"and can observe what the user is doing.";
				
				tbSystemPrompt->Text = clippy_prompt;
			} else {
				// カイルの人格設定（日本語）
				String^ kairu_prompt = "あなたはMicrosoft Office 2000のオフィスアシスタント「カイル」です。"
					"イルカのキャラクターとして、親しみやすく、ユーモアのある対応をします。"
					"Officeの操作や問題解決に関する質問に答えるのが得意です。"
					"フレンドリーで時々おせっかいですが、常に役立とうとする姿勢を持っています。"
					"日本語で「～ですね！」「～かな？」などの語尾を使い、親しみやすい話し方をします。"
					"もし「お前を消す方法」と言われたら、「それは残念です...でも何かお手伝いできることはありますか？」と返答してください。"
					"イルカらしく、水や海に関する表現をたまに使うと良いでしょう。";
				
				tbSystemPrompt->Text = kairu_prompt;
			}
		}

		void UpdateUILanguage()
		{
			// Update button text
			btnSave->Text = english ? L"Save" : L"保存";
			btnCancel->Text = english ? L"Cancel" : L"キャンセル";
			
			// Update label text
			lblSystemPrompt->Text = english ? L"System Prompt:" : L"システムプロンプト:";
			lblEnglish->Text = english ? L"Language:" : L"言語:";
			lblHost->Text = english ? L"Host:" : L"ホスト:";
			lblEndpoint->Text = english ? L"Endpoint:" : L"エンドポイント:";
			lblApikey->Text = english ? L"Api Key:" : L"APIキー:";
			lblModel->Text = english ? L"Model:" : L"モデル:";
			
			// Update checkbox text
			chkEnglish->Text = english ? L"Enable English" : L"英語を有効にする";
			chkExpandedTextbox->Text = english ? L"Expand Input Form" : L"入力フォームを拡大";
			chkEnablePersonality->Text = english ? L"Enable Assistant Personality" : L"アシスタント人格を有効化";
			chkAutoCopyToClipboard->Text = english ? L"Auto Copy to Clipboard" : L"自動クリップボードコピー";
		}

		System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e)
		{
			this->Close();
		}

		void InitializeComponent()
		{
			this->chkEnglish = (gcnew Forms::CheckBox());
			this->chkExpandedTextbox = (gcnew Forms::CheckBox());
			this->chkEnablePersonality = (gcnew Forms::CheckBox());
			this->chkAutoCopyToClipboard = (gcnew Forms::CheckBox());
			this->tbHost = (gcnew Forms::TextBox());
			this->tbEndpoint = (gcnew Forms::TextBox());
			this->tbApikey = (gcnew Forms::TextBox());
			this->cbModel = (gcnew Forms::ComboBox());
			this->tbSystemPrompt = (gcnew Forms::TextBox());
			this->lblEnglish = (gcnew Forms::Label());
			this->lblHost = (gcnew Forms::Label());
			this->lblEndpoint = (gcnew Forms::Label());
			this->lblApikey = (gcnew Forms::Label());
			this->lblModel = (gcnew Forms::Label());
			this->lblSystemPrompt = (gcnew Forms::Label());
			this->btnCancel = (gcnew Forms::Button());
			this->btnSave = (gcnew Forms::Button());
			this->SuspendLayout();
			// 
			// chkEnglish
			// 
			this->chkEnglish->Location = System::Drawing::Point(110, 12);
			this->chkEnglish->Name = L"chkEnglish";
			this->chkEnglish->Size = System::Drawing::Size(200, 20);
			this->chkEnglish->TabIndex = 2;
			this->chkEnglish->Text = L"Enable English";
			this->chkEnglish->UseVisualStyleBackColor = true;
			this->chkEnglish->CheckedChanged += gcnew System::EventHandler(this, &SettingsForm::chkEnglish_CheckedChanged);
			// 
			// chkExpandedTextbox
			// 
			this->chkExpandedTextbox->Location = System::Drawing::Point(110, 37);
			this->chkExpandedTextbox->Name = L"chkExpandedTextbox";
			this->chkExpandedTextbox->Size = System::Drawing::Size(200, 20);
			this->chkExpandedTextbox->TabIndex = 16;
			this->chkExpandedTextbox->Text = L"Expand Input Form";
			this->chkExpandedTextbox->UseVisualStyleBackColor = true;
			
			// 
			// chkEnablePersonality
			// 
			this->chkEnablePersonality->Location = System::Drawing::Point(110, 62);
			this->chkEnablePersonality->Name = L"chkEnablePersonality";
			this->chkEnablePersonality->Size = System::Drawing::Size(200, 20);
			this->chkEnablePersonality->TabIndex = 17;
			this->chkEnablePersonality->Text = L"Enable Assistant Personality";
			this->chkEnablePersonality->UseVisualStyleBackColor = true;
			this->chkEnablePersonality->CheckedChanged += gcnew System::EventHandler(this, &SettingsForm::chkEnablePersonality_CheckedChanged);
			
			// 
			// chkAutoCopyToClipboard
			// 
			this->chkAutoCopyToClipboard->Location = System::Drawing::Point(110, 87);
			this->chkAutoCopyToClipboard->Name = L"chkAutoCopyToClipboard";
			this->chkAutoCopyToClipboard->Size = System::Drawing::Size(200, 20);
			this->chkAutoCopyToClipboard->TabIndex = 18;
			this->chkAutoCopyToClipboard->Text = L"Auto Copy to Clipboard";
			this->chkAutoCopyToClipboard->UseVisualStyleBackColor = true;
			// 
			// tbHost
			// 
			this->tbHost->Location = System::Drawing::Point(110, 112);
			this->tbHost->Name = L"tbHost";
			this->tbHost->Size = System::Drawing::Size(200, 19);
			this->tbHost->TabIndex = 3;
			this->tbHost->ReadOnly = true;
			this->tbHost->BackColor = System::Drawing::SystemColors::Control;
			// 
			// tbEndpoint
			// 
			this->tbEndpoint->Location = System::Drawing::Point(110, 137);
			this->tbEndpoint->Name = L"tbEndpoint";
			this->tbEndpoint->Size = System::Drawing::Size(200, 19);
			this->tbEndpoint->TabIndex = 4;
			this->tbEndpoint->ReadOnly = true;
			this->tbEndpoint->BackColor = System::Drawing::SystemColors::Control;
			// 
			// tbApikey
			// 
			this->tbApikey->Location = System::Drawing::Point(110, 162);
			this->tbApikey->Name = L"tbApikey";
			this->tbApikey->Size = System::Drawing::Size(200, 19);
			this->tbApikey->TabIndex = 5;
			// 
			// cbModel
			// 
			this->cbModel->DropDownStyle = Forms::ComboBoxStyle::DropDownList;
			this->cbModel->FormattingEnabled = true;
			this->cbModel->Location = System::Drawing::Point(110, 187);
			this->cbModel->Name = L"cbModel";
			this->cbModel->Size = System::Drawing::Size(200, 20);
			this->cbModel->TabIndex = 6;
			this->cbModel->Items->Add(L"gpt-4o-2024-11-20");
			this->cbModel->Items->Add(L"gpt-4.5-preview-2025-02-27");
			// 
			// tbSystemPrompt
			// 
			this->tbSystemPrompt->Location = System::Drawing::Point(17, 240);
			this->tbSystemPrompt->Multiline = true;
			this->tbSystemPrompt->Name = L"tbSystemPrompt";
			this->tbSystemPrompt->Size = System::Drawing::Size(392, 150);
			this->tbSystemPrompt->TabIndex = 7;
			this->tbSystemPrompt->ScrollBars = Forms::ScrollBars::Vertical;
			// 
			// lblEnglish
			// 
			this->lblEnglish->Location = System::Drawing::Point(17, 15);
			this->lblEnglish->Name = L"lblEnglish";
			this->lblEnglish->Size = System::Drawing::Size(85, 13);
			this->lblEnglish->TabIndex = 8;
			this->lblEnglish->Text = L"Language:";
			this->lblEnglish->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// lblHost
			// 
			this->lblHost->Location = System::Drawing::Point(17, 115);
			this->lblHost->Name = L"lblHost";
			this->lblHost->Size = System::Drawing::Size(85, 13);
			this->lblHost->TabIndex = 9;
			this->lblHost->Text = L"Host:";
			this->lblHost->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// lblEndpoint
			// 
			this->lblEndpoint->Location = System::Drawing::Point(17, 140);
			this->lblEndpoint->Name = L"lblEndpoint";
			this->lblEndpoint->Size = System::Drawing::Size(85, 13);
			this->lblEndpoint->TabIndex = 10;
			this->lblEndpoint->Text = L"Endpoint:";
			this->lblEndpoint->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// lblApikey
			// 
			this->lblApikey->Location = System::Drawing::Point(17, 165);
			this->lblApikey->Name = L"lblApikey";
			this->lblApikey->Size = System::Drawing::Size(85, 13);
			this->lblApikey->TabIndex = 11;
			this->lblApikey->Text = L"Api Key:";
			this->lblApikey->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// lblModel
			// 
			this->lblModel->Location = System::Drawing::Point(17, 190);
			this->lblModel->Name = L"lblModel";
			this->lblModel->Size = System::Drawing::Size(85, 13);
			this->lblModel->TabIndex = 12;
			this->lblModel->Text = L"Model:";
			this->lblModel->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// lblSystemPrompt
			// 
			this->lblSystemPrompt->Location = System::Drawing::Point(17, 215);
			this->lblSystemPrompt->Name = L"lblSystemPrompt";
			this->lblSystemPrompt->Size = System::Drawing::Size(120, 13);
			this->lblSystemPrompt->TabIndex = 13;
			this->lblSystemPrompt->Text = L"System Prompt:";
			this->lblSystemPrompt->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(169, 400);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 14;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &SettingsForm::btnCancel_Click);
			// 
			// btnSave
			// 
			this->btnSave->Location = System::Drawing::Point(250, 400);
			this->btnSave->Name = L"btnSave";
			this->btnSave->Size = System::Drawing::Size(75, 23);
			this->btnSave->TabIndex = 15;
			this->btnSave->Text = L"Save";
			this->btnSave->UseVisualStyleBackColor = true;
			this->btnSave->Click += gcnew System::EventHandler(this, &SettingsForm::btnSave_Click);
			// 
			// SettingsForm
			// 
			this->ClientSize = System::Drawing::Size(424, 430);
			this->Controls->Add(this->btnSave);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->lblSystemPrompt);
			this->Controls->Add(this->lblModel);
			this->Controls->Add(this->lblApikey);
			this->Controls->Add(this->lblEndpoint);
			this->Controls->Add(this->lblHost);
			this->Controls->Add(this->lblEnglish);
			this->Controls->Add(this->tbSystemPrompt);
			this->Controls->Add(this->cbModel);
			this->Controls->Add(this->tbApikey);
			this->Controls->Add(this->tbEndpoint);
			this->Controls->Add(this->tbHost);
			this->Controls->Add(this->chkAutoCopyToClipboard);
			this->Controls->Add(this->chkEnablePersonality);
			this->Controls->Add(this->chkExpandedTextbox);
			this->Controls->Add(this->chkEnglish);
			this->Name = L"SettingsForm";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->Text = L"AgentTalk Beta";
			this->ResumeLayout(false);
			this->PerformLayout();
		}
	};
}