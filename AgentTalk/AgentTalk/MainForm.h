// MainForm.h
#pragma once
#include "Worker.h"
#include "SettingsForm.h"
#include "IniFile.h"
#include <ctime>

[System::Runtime::InteropServices::DllImportAttribute("user32.dll", CharSet = System::Runtime::InteropServices::CharSet::Auto, SetLastError = true)]
extern "C" short GetAsyncKeyState(System::UInt16 virtualKeyCode);

namespace AgentTalk {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;
	using namespace System::Media;

	public ref class MainForm : public System::Windows::Forms::Form
	{
	private:
		bool english;
		bool expandedTextbox;
		bool enablePersonality;
		bool autoCopyToClipboard; // �񓚂������I�ɃN���b�v�{�[�h�ɃR�s�[����t���O
		IniFile^ ini;
		System::Collections::Specialized::NameValueCollection^ values;
		ToolStripMenuItem^ alwaysOnTopMenuItem;
		BackgroundWorker^ worker;
		System::ComponentModel::Container^ components;
		bool ctrlCFirstPress;
		clock_t ctrlCFirstTime;
    System::Windows::Forms::Timer^ imageTimer;
    System::String^ secondImagePath;
		System::Windows::Forms::Timer^ timer;
		System::Windows::Forms::ToolTip^ toolTip;

	public:
		MainForm(void)
		{
			ctrlCFirstPress = false;
			timer = gcnew System::Windows::Forms::Timer();
			ini = gcnew IniFile();
			values = ini->ReadAllValues("agenttalk.ini");
			english = (values["english"]->ToLower() == "true");
			
			// �g��ݒ��ǂݍ���
			expandedTextbox = false;
			if (values["expandedTextbox"] != nullptr) 
			{
				expandedTextbox = (values["expandedTextbox"]->ToLower() == "true");
			}
			
			// �l�i�ݒ��ǂݍ���
			enablePersonality = false;
			if (values["enablePersonality"] != nullptr) 
			{
				enablePersonality = (values["enablePersonality"]->ToLower() == "true");
			}
			
			// �����N���b�v�{�[�h�R�s�[�ݒ��ǂݍ���
			autoCopyToClipboard = false;
			if (values["autoCopyToClipboard"] != nullptr) 
			{
				autoCopyToClipboard = (values["autoCopyToClipboard"]->ToLower() == "true");
			}
			
			InitializeComponent();
			this->pictureBox1->Location = Point(english ? this->pictureBox1->Location.X - 8 : this->pictureBox1->Location.X - 2, english ? this->pictureBox1->Location.Y : this->pictureBox1->Location.Y - 10);
			this->pictureBox1->Size = System::Drawing::Size(this->pictureBox1->Size.Width, this->pictureBox1->Size.Height + 30);
			System::String^  imagePath = english ? "clippy_start.gif" : "kairu_start.gif";
			this->pictureBox1->Image = System::Drawing::Image::FromFile(imagePath);
			System::String^ wavPath = english ? "clippy.wav" : "kairu.wav";
			SoundPlayer^ sound = gcnew SoundPlayer(wavPath);
			sound->Play(); 

			secondImagePath = english ? "clippy.gif" : "kairu.gif";
	    
			imageTimer = gcnew System::Windows::Forms::Timer();
			imageTimer->Interval = 1400;
			imageTimer->Tick += gcnew System::EventHandler(this, &MainForm::OnImageTimerTick);
			imageTimer->Start();
    
			this->components = gcnew System::ComponentModel::Container();
			this->notifyIcon1 = (gcnew System::Windows::Forms::NotifyIcon(this->components));
			this->notifyIcon1->Icon = System::Drawing::Icon::ExtractAssociatedIcon("app.ico");
			this->notifyIcon1->Text = "Agent Talk";
			this->notifyIcon1->Visible = true;
			this->notifyIcon1->ContextMenuStrip = (gcnew System::Windows::Forms::ContextMenuStrip());
			this->alwaysOnTopMenuItem = gcnew ToolStripMenuItem(english ? "Always On Top" : "��Ɏ�O�ɕ\��", nullptr, gcnew EventHandler(this, &MainForm::AlwaysOnTopMenuItem_Click));
			this->alwaysOnTopMenuItem->Checked = true;
			this->notifyIcon1->ContextMenuStrip->Items->Add(this->alwaysOnTopMenuItem);
			this->notifyIcon1->ContextMenuStrip->Items->Add(english ? "Exit" : "�I��", nullptr, gcnew EventHandler(this, &MainForm::ExitMenuItem_Click));
			this->ShowInTaskbar = false;
			this->TopMost = true;
			this->TransparencyKey = Color::Magenta;
			this->FormBorderStyle = Windows::Forms::FormBorderStyle::None;
			this->StartPosition = Windows::Forms::FormStartPosition::Manual;
			this->pictureBox1->DoubleClick += gcnew System::EventHandler(this, &MainForm::pictureBox1_DoubleClick);
			this->pictureBox1->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox1_MouseDown);
			this->pictureBox1->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox1_MouseMove);
			this->button1->Click += gcnew System::EventHandler(this, &MainForm::button1_Click);
			this->button2->Click += gcnew System::EventHandler(this, &MainForm::button2_Click);

			this->timer->Interval = 200;
			this->timer->Tick += gcnew System::EventHandler(this, &MainForm::timer_Tick);
			this->timer->Enabled = true;
			this->StartPosition = FormStartPosition::Manual;
			Rectangle workingArea = System::Windows::Forms::Screen::PrimaryScreen->WorkingArea;
			int width = this->Width;
			int height = this->Height;
			this->Location = Point(workingArea.Right - width - 20, workingArea.Bottom - height - 20);

			worker = gcnew BackgroundWorker;
			worker->DoWork += gcnew DoWorkEventHandler(this, &MainForm::worker_DoWork);
			worker->RunWorkerCompleted += gcnew RunWorkerCompletedEventHandler(this, &MainForm::worker_RunWorkerCompleted);
			
			// �c�[���`�b�v��������
			this->toolTip = gcnew System::Windows::Forms::ToolTip();
			UpdateButtonTooltips();
			
			// �e�L�X�g�{�b�N�X�ɃL�[�����C�x���g��ǉ�
			this->textBox1->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MainForm::textBox1_KeyDown);
			
			// �g��ݒ�ɉ����ăs�N�`���[�{�b�N�X�ƃe�L�X�g�{�b�N�X�̑傫���𒲐�
			AdjustTextboxSize();
		}
	protected:
		~MainForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		// �e�L�X�g�{�b�N�X�̃T�C�Y�𒲐����郁�\�b�h
		void AdjustTextboxSize()
		{
			if (expandedTextbox)
			{
				// pictureBox3���c��1.3�{�ɂ���i������Ɋg���j
				int originalHeight = this->pictureBox3->Height;
				int newHeight = (int)(originalHeight * 1.3);
				
				// ������Ɋg�����邽�߁AY���W����ɃV�t�g
				this->pictureBox3->Location = System::Drawing::Point(
					this->pictureBox3->Location.X, 
					this->pictureBox3->Location.Y - (newHeight - originalHeight)
				);
				
				// �T�C�Y��ύX
				this->pictureBox3->Size = System::Drawing::Size(this->pictureBox3->Width, newHeight);
				
				// pictureBox2�i�㕔�̌r���j����Ɉړ�
				this->pictureBox2->Location = System::Drawing::Point(
					this->pictureBox2->Location.X,
					this->pictureBox3->Location.Y - this->pictureBox2->Height
				);
				
				// �e�L�X�g�{�b�N�X����Ɉړ����A�����𒲐��i�{�^���Əd�Ȃ�Ȃ��悤�Z���j
				this->textBox1->Location = System::Drawing::Point(
					this->textBox1->Location.X,
					this->pictureBox3->Location.Y + 3  // ���x�����Ȃ��̂Ńt�H�[���㕔�ɋ߂Â���
				);
				
				// �{�^���Ƃ̊Ԃɗ]�����m�ۂ��邽�߁A�����𒲐�
				this->textBox1->Size = System::Drawing::Size(
					this->textBox1->Width, 
					newHeight - 14  // �]����14�Ɏw��
				);
				
				// ���x�����\���ɂ���
				this->label1->Visible = false;
				
				// �p�l���̈ʒu�ƃT�C�Y�������ipictureBox2���܂߂�悤�Ɂj
				this->panel1->Location = System::Drawing::Point(
					this->panel1->Location.X,
					this->pictureBox2->Location.Y
				);
				this->panel1->Size = System::Drawing::Size(
					this->panel1->Width,
					this->pictureBox4->Location.Y + this->pictureBox4->Height - this->panel1->Location.Y
				);
			}
			else
			{
				// �ʏ탂�[�h�ɖ߂��ꍇ�̓��x����\��
				this->label1->Visible = true;
			}
		}

		void OnImageTimerTick(Object^ sender, EventArgs^ e)
		{
				this->pictureBox1->Location = Point(english ? this->pictureBox1->Location.X + 8 : this->pictureBox1->Location.X + 2, english ? this->pictureBox1->Location.Y : this->pictureBox1->Location.Y + 10);
				this->pictureBox1->Image = gcnew System::Drawing::Bitmap(secondImagePath);
				imageTimer->Stop();
		}
		void timer_Tick(Object^ sender, EventArgs^ e)
		{
			if (GetAsyncKeyState(0x11) != 0) // Check if Control key is pressed
			{
				if (GetAsyncKeyState(0x43) != 0) // Check if 'C' key is pressed
				{
					if (!ctrlCFirstPress)
					{
						ctrlCFirstPress = true;
						ctrlCFirstTime = clock();
					}
					else
					{
						double elapsed = double(clock() - ctrlCFirstTime) / CLOCKS_PER_SEC;
						if (elapsed <= 1.0)
						{
							int panel1Index = this->Controls->GetChildIndex(panel1);
							int highestZIndex = 0;
							for each (Control^ control in this->Controls)
							{
								int currentZIndex = this->Controls->GetChildIndex(control);
								if (currentZIndex < highestZIndex)
								{
									highestZIndex = currentZIndex;
								}
							}

							if (panel1Index == highestZIndex)
							{
								pictureBox1_DoubleClick(sender, e);
							}
							if (button2->Text == "Reset" || button2->Text == "���Z�b�g")
							{
								button2_Click(sender, e);
							}

							textBox1->Text = Clipboard::GetText();

							System::Windows::Forms::Timer^ delayTimer = gcnew System::Windows::Forms::Timer();
							delayTimer->Interval = 500;
							delayTimer->Tick += gcnew System::EventHandler(this, &AgentTalk::MainForm::OnDelayTimerTick);
							delayTimer->Start();
						}
						ctrlCFirstPress = false;
					}
				}
			}
			else
			{
				ctrlCFirstPress = false;
			}
		}

		void OnDelayTimerTick(Object^ sender, EventArgs^ e)
		{
			((System::Windows::Forms::Timer^)sender)->Stop();

			button1->PerformClick();
		}

		// �e�L�X�g�{�b�N�X�̃L�[�����C�x���g������
		void textBox1_KeyDown(Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
		{
			// Ctrl+Enter�������ꂽ�猻�݂̃A�N�e�B�u�{�^���̃N���b�N���V�~�����[�g
			if (e->Control && e->KeyCode == Keys::Enter)
			{
				e->SuppressKeyPress = true; // Enter�L�[�̒ʏ�̏�����}��
				
				// ���M�{�^�����L���ȏꍇ�̂ݏ���
				if (this->button1->Enabled)
				{
					// �{�^���̃e�L�X�g�ɉ����ď����𕪊�
					if (this->button1->Text == (english ? "Continue" : "������") || 
						this->button1->Text == (english ? "Send" : "���M"))
					{
						this->button1->PerformClick();
					}
				}
			}
			// Ctrl+Backspace�������ꂽ�烊�Z�b�g�{�^���̃N���b�N���V�~�����[�g
			else if (e->Control && e->KeyCode == Keys::Back)
			{
				e->SuppressKeyPress = true; // Backspace�L�[�̒ʏ�̏�����}��
				
				// ���Z�b�g�{�^�������Z�b�g��Ԃ̏ꍇ�̂ݏ���
				if (this->button2->Text == (english ? "Reset" : "���Z�b�g"))
				{
					this->button2->PerformClick();
				}
			}
			// Ctrl+A�������ꂽ��e�L�X�g�S�I��
			else if (e->Control && e->KeyCode == Keys::A)
			{
				// �e�L�X�g�{�b�N�X���ҏW�\�ȏꍇ�̂ݑS�I��
				if (!this->textBox1->ReadOnly)
				{
					this->textBox1->SelectAll();
					e->SuppressKeyPress = true; // A�L�[�̒ʏ�̏�����}��
				}
			}
		}

		System::Void AlwaysOnTopMenuItem_Click(System::Object^ sender, System::EventArgs^ e)
		{
			this->TopMost = !this->TopMost;
			alwaysOnTopMenuItem->Checked = !alwaysOnTopMenuItem->Checked;
		}

	private: 
		PictureBox^ pictureBox1;
		PictureBox^ pictureBox2;
		PictureBox^ pictureBox3;
		PictureBox^ pictureBox4;
		Label^ label1;
		TextBox^ textBox1;
		NotifyIcon^ notifyIcon1;
		Button^ button1;
		Button^ button2;
		Panel^ panel1;
		System::Drawing::Point^ lastPoint;


	private: 
		void MainForm_Load(Object^ sender, EventArgs^ e) {}
		void ExitMenuItem_Click(Object^ sender, EventArgs^ e) { Application::Exit(); }

		void pictureBox1_MouseDown(Object^ sender, MouseEventArgs^ e) { lastPoint = e->Location; }

		void pictureBox1_MouseMove(Object^ sender, MouseEventArgs^ e) {
			if (e->Button == Windows::Forms::MouseButtons::Left)
			{
				this->Left += e->X - lastPoint->X;
				this->Top += e->Y - lastPoint->Y;
			}
		}
	public:
		void button1_Click(Object^ sender, EventArgs^ e) 
		{
			// �{�^���̃e�L�X�g���uContinue�v���uSearch�v���ɂ���ď����𕪂���
			bool isContinue = (this->button1->Text == (english ? "Continue" : "������"));
			
			if (isContinue) {
				// �u������v���[�h�̏ꍇ�̓e�L�X�g�{�b�N�X���N���A���A
				// �ǉ��w������͂ł���悤�ɂ���
				this->textBox1->Clear();
				this->textBox1->ReadOnly = false;
				this->textBox1->BackColor = Color::White;
				this->button1->Text = english ? "Send" : "���M";
				this->textBox1->Focus();
				
				// �c�[���`�b�v���X�V
				UpdateButtonTooltips();
				return; // �������I��
			}
			
			// �uSend�v�܂��́uSearch�v�̏ꍇ�͏��������s
			this->button1->Text = english ? "Processing..." : "������...";
			SoundPlayer^ sound = gcnew SoundPlayer("menucommand.wav");
			sound->PlaySync();
			this->button1->Enabled = false;
			
			String^ question = this->textBox1->Text;
			
			// ���͂���̏ꍇ�͏������Ȃ�
			if (String::IsNullOrEmpty(question) || question->Trim()->Length == 0) {
				this->button1->Enabled = true;
				this->button1->Text = english ? "Send" : "���M";
				return;
			}
			
			this->textBox1->ReadOnly = true;
			this->textBox1->BackColor = Color::FromArgb(255, 255, 206);

			worker->RunWorkerAsync(question);
		}

		void worker_DoWork(Object^ sender, DoWorkEventArgs^ e) 
		{
			String^ question = dynamic_cast<String^>(e->Argument);
			Worker^ worker = gcnew Worker(question);
			worker->AskGPT();
			e->Result = worker->result; // pass the result to the RunWorkerCompleted event handler
		}

		// �{�^���̃c�[���`�b�v���X�V
		void UpdateButtonTooltips()
		{
			if (this->button1->Text == (english ? "Continue" : "������"))
			{
				this->toolTip->SetToolTip(this->button1, english ? "Continue (Ctrl+Enter)" : "������ (Ctrl+Enter)");
			}
			else
			{
				this->toolTip->SetToolTip(this->button1, english ? "Send (Ctrl+Enter)" : "���M (Ctrl+Enter)");
			}
			
			// ���Z�b�g�{�^���̃c�[���`�b�v
			if (this->button2->Text == (english ? "Reset" : "���Z�b�g"))
			{
				this->toolTip->SetToolTip(this->button2, english ? "Reset (Ctrl+Backspace)" : "���Z�b�g (Ctrl+Backspace)");
			}
			else
			{
				this->toolTip->SetToolTip(this->button2, english ? "Options" : "�I�v�V����");
			}
		}

		void worker_RunWorkerCompleted(Object^ sender, RunWorkerCompletedEventArgs^ e) 
		{
			if (e->Error != nullptr) {
			} else {
				String^ response = dynamic_cast<String^>(e->Result);

				this->label1->Text = english ? "Here is the response!" : "�����ł��I";
				this->textBox1->Text = response;
				this->button2->Text = english ? "Reset" : "���Z�b�g";
				this->button1->Enabled = true;
				this->button1->Text = english ? "Continue" : "������";
				
				// �ԐM��̓e�L�X�g�{�b�N�X��ǂݎ���p�ɐݒ�
				this->textBox1->ReadOnly = true;
				
				// �����N���b�v�{�[�h�R�s�[���L���ȏꍇ�A�������N���b�v�{�[�h�ɃR�s�[
				if (autoCopyToClipboard && !String::IsNullOrEmpty(response)) {
					try {
						Clipboard::SetText(response);
					}
					catch (Exception^) {
						// �N���b�v�{�[�h����ŃG���[���������Ă����s
					}
				}
				
				// �c�[���`�b�v���X�V
				UpdateButtonTooltips();
			}
		}

		void button2_Click(Object^ sender, EventArgs^ e) {
			if(this->button2->Text == "Reset" || this->button2->Text == "���Z�b�g"){
				this->textBox1->Text = english ? "Type your question here, and then click Send." : "�����Ɏ��╶����͂��A[���M] ���N���b�N���Ă��������I";
				this->label1->Text = english ? "What would you like to do?" : "���ɂ��Ē��ׂ܂����H";
				this->textBox1->ReadOnly = false;
				this->textBox1->BackColor = Color::White;
				this->button2->Text = english ? "Options" : "�I�v�V����";
				this->button1->Text = english ? "Send" : "���M";
				
				// ��b���������Z�b�g
				Worker::ResetConversation();
				
				// �c�[���`�b�v���X�V
				UpdateButtonTooltips();
			}
			else{
				SettingsForm^ settingsForm = gcnew SettingsForm(values);
				settingsForm->ShowDialog();
			}
		}

	private: 
		void pictureBox1_DoubleClick(Object^ sender, EventArgs^ e) {
			bool panel1_is_on_top = true;
			int panel1_index = -1;
			for (int i = 0; i < this->Controls->Count; ++i)
			{
				Control^ control = this->Controls[i];
				if (control == this->panel1)
				{
					panel1_index = i;
					break;
				}
			}
			if (panel1_index == this->Controls->Count - 1) panel1_is_on_top = false;
			if (panel1_is_on_top)
			{
				this->panel1->SendToBack();
			}
			else
			{
				this->panel1->BringToFront();
				this->textBox1->SelectAll();
			}
		}

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			ComponentResourceManager^  resources = gcnew ComponentResourceManager(MainForm::typeid);
			this->pictureBox1 = gcnew PictureBox();
			this->pictureBox2 = gcnew PictureBox();
			this->pictureBox3 = gcnew PictureBox();
			this->pictureBox4 = gcnew PictureBox();
			this->label1 = gcnew Label();
			this->textBox1 = gcnew TextBox();
			this->button1 = gcnew Button();
			this->button2 = gcnew Button();
			this->panel1 = gcnew Panel();
			this->SuspendLayout();

			this->pictureBox1->Image = safe_cast<Drawing::Image^>(resources->GetObject("pictureBox1.Image"));
			this->pictureBox1->Location = Drawing::Point(158, 173);
			this->pictureBox1->Name = "pictureBox1";
			this->pictureBox1->Size = Drawing::Size(100, 78);
			this->pictureBox1->TabIndex = 0;
			this->pictureBox1->TabStop = false;

			this->pictureBox2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject("pictureBox2.Image")));
			this->pictureBox2->Location = System::Drawing::Point(44, 27);
			this->pictureBox2->Name = "pictureBox2";
			this->pictureBox2->Size = System::Drawing::Size(226, 9);
			this->pictureBox2->TabIndex = 1;
			this->pictureBox2->TabStop = false;

			this->pictureBox3->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject("pictureBox3.Image")));
			this->pictureBox3->Location = System::Drawing::Point(44, 36);
			this->pictureBox3->Name = "pictureBox3";
			this->pictureBox3->Size = System::Drawing::Size(226, 83);
			this->pictureBox3->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox3->TabIndex = 2;
			this->pictureBox3->TabStop = false;

			this->pictureBox4->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject("pictureBox4.Image")));
			this->pictureBox4->Location = System::Drawing::Point(44, 113);
			this->pictureBox4->Name = "pictureBox4";
			this->pictureBox4->Size = System::Drawing::Size(226, 50);
			this->pictureBox4->TabIndex= 3;
			this->pictureBox4->TabStop = false;

			this->label1->AutoSize = true;
			this->label1->BackColor = Color::FromArgb(255, 255, 214);
			this->label1->Font = gcnew Drawing::Font("MS UI Gothic", 11.25F, FontStyle::Regular, Drawing::GraphicsUnit::Point, 128);
			this->label1->Location = Drawing::Point(53, 34);
			this->label1->Name = "label1";
			this->label1->Size = Drawing::Size(148, 15);
			this->label1->TabIndex = 4;
			this->label1->Text = english ? "What would you like to do?" : "���ɂ��Ē��ׂ܂����H";

			this->textBox1->BackColor = SystemColors::Window;
			this->textBox1->BorderStyle = Windows::Forms::BorderStyle::None;
			this->textBox1->Font = gcnew Drawing::Font("MS UI Gothic", 9, FontStyle::Regular, Drawing::GraphicsUnit::Point, 128);
			this->textBox1->ForeColor = SystemColors::WindowText;
			this->textBox1->Location = Drawing::Point(56, 55);
			this->textBox1->Multiline = true;
			this->textBox1->Name = "textBox1";
			this->textBox1->ScrollBars = Windows::Forms::ScrollBars::Vertical;
			this->textBox1->Size = Drawing::Size(202, 52);
			this->textBox1->TabIndex = 5;
			this->textBox1->Text = english ? "Type your question here, and then click Send." : "�����Ɏ��╶����͂��A[���M] ���N���b�N���Ă��������I";

			this->button1->BackColor = Color::FromArgb(255, 255, 206);
			this->button1->FlatAppearance->BorderSize = 0;
			this->button1->FlatStyle = Windows::Forms::FlatStyle::Flat;
			this->button1->Font = gcnew Drawing::Font("MS UI Gothic", 8.25F, FontStyle::Regular, Drawing::GraphicsUnit::Point, 128);
			this->button1->Location = Drawing::Point(183, 118);
			this->button1->Margin = Windows::Forms::Padding(0);
			this->button1->Name = "button1";
			this->button1->Size = Drawing::Size(75, 18);
			this->button1->TabIndex = 6;
			this->button1->Text = english ? "Send" : "���M";
			this->button1->UseVisualStyleBackColor = false;

			this->button2->BackColor = Color::FromArgb(255, 255, 206);
			this->button2->FlatAppearance->BorderSize = 0;
			this->button2->FlatStyle = Windows::Forms::FlatStyle::Flat;
			this->button2->Font = gcnew Drawing::Font("MS UI Gothic", 8.25F, FontStyle::Regular, Drawing::GraphicsUnit::Point, 128);
			this->button2->Location = Drawing::Point(56, 118);
			this->button2->Margin = Windows::Forms::Padding(0);
			this->button2->Name = "button2";
			this->button2->Size = Drawing::Size(75, 18);
			this->button2->TabIndex = 7;
			this->button2->Text = english ? "Options" : "�I�v�V����";
			this->button2->UseVisualStyleBackColor = false;

			this->panel1->Location = Drawing::Point(22, 13);
			this->panel1->Name = "panel1";
			this->panel1->Size = Drawing::Size(258, 150);
			this->panel1->TabIndex = 8;

			this->AutoScaleDimensions = Drawing::SizeF(6, 12);
			this->AutoScaleMode = Windows::Forms::AutoScaleMode::Font;
			this->BackColor = Color::Magenta;
			this->ClientSize = Drawing::Size(292, 266);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->pictureBox4);
			this->Controls->Add(this->pictureBox3);
			this->Controls->Add(this->pictureBox2);
			this->Controls->Add(this->pictureBox1);           
			this->Name = "MainForm";
			this->Text = "MainForm";
			this->Load += gcnew EventHandler(this, &MainForm::MainForm_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox2))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox3))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox4))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();
		}
#pragma endregion
	};
}