void Callback(char* data, int size)
{
	if(!jtPts.ParseFromArray(data, size) )	{
		cerr<<"Deserialization error."<<endl;
		return;
	}
	disp_trajectory();
	//generate pulse according to trajectory
	generate_pulses();
}

void generate_pulses()
{
	double param; 	//100 us
	int tmSpendInUs; 	//us
	int curPosInPulses=0, pulses_needed,cmdPosInPulses;
	int pulse_width; 	//us
	//得到脉冲接口
	RpiAxis * rpiAxis=RpiAxis::getInstance();
	rpiAxis->init(1000);

	google::protobuf::RepeatedPtrField<const Point>::iterator prev_pt_iter, cur_pt_iter;
	prev_pt_iter=jtPts.points().begin();
	for(cur_pt_iter=prev_pt_iter+1; cur_pt_iter!=jtPts.points().end(); prev_pt_iter=cur_pt_iter, cur_pt_iter++ )
	{
		//计算时间段的时间
		tmSpendInUs=(int)((cur_pt_iter->time_frome_start()-prev_pt_iter->time_frome_start())*1000*1000);
		//calculate how many pulses need to send. do unit conversion
		//将以弧度表示的位置转换为所需发送的脉冲数，其中1.8*2为细分的单个脉冲角度，36为减速器倍数。
		cmdPosInPulses=(int)(cur_pt_iter->positions(0)*(360.0/3.1415926)/(1.8*2)* 36);
		pulses_needed=cmdPosInPulses-curPosInPulses;
		cout<<"pulses: "<<pulses_needed<<", ";
		//确定单个脉冲的宽度
		pulse_width=tmSpendInUs/pulses_needed;
		cout<<"pulse width: "<<pulse_width<<endl;
		//通过脉冲接口发送
		param=abs(pulse_width);
		rpiAxis->setParam(0,AxisMode::AXISMODE_PULSE_WITh_FIXED_WIDTH, &param, 1);
		rpiAxis->setCmd(0,pulses_needed);
		rpiAxis->startCmd();
		while(!rpiAxis->isFinished()) ; 	//等待发送结束
		//更新位置
		curPosInPulses =cmdPosInPulses;
	}
}
