% Beam-based Feedback Parameters Configuration
% Jack Olivieri - LBNL: February 2012
%%% measurements %%%
% 1st row -> Energy deviations
% 2nd row -> Bunch length deviations
% 3rd row -> time deviations
% 4th row -> relative energy spread
% more rows? maybe in the future
% columns -> input signal to dc matrix
% element value -> linac
%%% corrections %%%
% 1st row -> amplitude errors
% 2nd row -> phase errors
% columns -> output signal to dc matrix
% element value -> linac

function [bbf] = bbf_config(params_doublecomp, charge_nom,g)


% Select matrix configuration (default = m6x6)
m20x10 = 1; m20x10b = 0; m20x8 = 0; m15x10 = 0; m15x10b = 0; m15x8 = 0; m7x7 = 0; m6x6 = 0;

if(m20x10) % dE_E, sz, dt, sd and all actuators
	bbf.measurements = 	[1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0;
				0,1,0,0, 0,2,0,0, 0,3,0,0, 0,4,0,0, 0,5,0,0;
				0,0,1,0, 0,0,2,0, 0,0,3,0, 0,0,4,0, 0,0,5,0;
				0,0,0,1, 0,0,0,2, 0,0,0,3, 0,0,0,4, 0,0,0,5];
	bbf.corrections = [1,0,2,0,3,0,4,0,5,0; 0,1,0,2,0,3,0,4,0,5];
	ki = g*ones(size(bbf.corrections,2),1);
end

if(m20x10b) % dE_E, sz, dt, sd and all actuators with last four gains disabled
	bbf.measurements = 	[1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0;
				0,1,0,0, 0,2,0,0, 0,3,0,0, 0,4,0,0, 0,5,0,0;
				0,0,1,0, 0,0,2,0, 0,0,3,0, 0,0,4,0, 0,0,5,0;
				0,0,0,1, 0,0,0,2, 0,0,0,3, 0,0,0,4, 0,0,0,5];
	bbf.corrections = [1,0,2,0,3,0,4,0,5,0; 0,1,0,2,0,3,0,4,0,5];
	ki = [g; g; g; g; g; g; 0; 0; 0; 0];
end

if(m20x8) % dE_E, sz, dt, sd and all actuators except those of Lh
	bbf.measurements = 	[1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0;
				0,1,0,0, 0,2,0,0, 0,3,0,0, 0,4,0,0, 0,5,0,0;
				0,0,1,0, 0,0,2,0, 0,0,3,0, 0,0,4,0, 0,0,5,0;
				0,0,0,1, 0,0,0,5, 0,0,0,5, 0,0,0,5, 0,0,0,5];
	bbf.corrections = [1,0,2,0,4,0,5,0; 0,1,0,2,0,4,0,5];
	ki = g*ones(size(bbf.corrections,2),1);
end

if(m15x10) % dE, sz, and dt measurements for every linac section and all amplitude and phases
	bbf.measurements = 	[1,0,0, 2,0,0, 3,0,0, 4,0,0, 5,0,0;
				0,1,0, 0,2,0, 0,3,0, 0,4,0, 0,5,0;
				0,0,1, 0,0,2, 0,0,3, 0,0,4, 0,0,5];
	bbf.corrections = [1,0,2,0,3,0,4,0,5,0; 0,1,0,2,0,3,0,4,0,5];
	ki = g*ones(size(bbf.corrections,2),1)*0;
endif

if(m15x10b) % dE, sz, and dt measurements for every linac section and all amplitude and phases
	bbf.measurements = 	[0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0;
				1,0,0, 2,0,0, 3,0,0, 4,0,0, 5,0,0;
				0,1,0, 0,2,0, 0,3,0, 0,4,0, 0,5,0;
				0,0,1, 0,0,2, 0,0,3, 0,0,4, 0,0,5];
	bbf.corrections = [1,0,2,0,3,0,4,0,5,0; 0,1,0,2,0,3,0,4,0,5];
	ki = g*ones(size(bbf.corrections,2),1);
endif

if(m15x8) % dE, sz, and dt measurements for every linac section and all amplitude and phases
	bbf.measurements = 	[1,0,0, 2,0,0, 3,0,0, 4,0,0, 5,0,0;
				0,1,0, 0,2,0, 0,3,0, 0,4,0, 0,5,0;
				0,0,1, 0,0,2, 0,0,3, 0,0,4, 0,0,5];
	bbf.corrections = [1,0,2,0,4,0,5,0; 0,1,0,2,0,4,0,5];
	ki = g*ones(size(bbf.corrections,2),1);
endif

if(m7x7) % dE (L0,L1,L2,L3), sz (L0,Lh,L2); dV (L0,L1,L2,L3), dPhi (L0,L1,L2)
	bbf.measurements = [1,0,3,0,4,0,5; 0,1,0,3,0,4,0; 0,0,0,0,0,0,0; 0,0,0,0,0,0,0];
	bbf.corrections = [1,0,2,0,4,0,5; 0,1,0,2,0,4,0];
	ki = [g; g; g; g; g; g; g];
end

if(m6x6) % Standard dE (L0,L1,L2,L3), sz (Lh,L2); dV (L0,L1,L2,L3), dPhi (L1,L2)
	bbf.measurements = [1,3,0,4,0,5; 0,0,3,0,4,0; 0,0,0,0,0,0; 0,0,0,0,0,0];
	bbf.corrections = [1,2,0,4,0,5; 0,0,2,0,4,0];
	ki = [g; g; g; g; g; g];
end

    for k=1:size(bbf.measurements,2)
        if (length(find(bbf.measurements(:,k)))>1)
            error(['Overlapping of BBFB measurement signals ' ...
                   'in feedback vectors\nCheck BBFB measurements ' ...
                   'configuration']);
        end;
    end
    for k=1:size(bbf.corrections,2)
        if (length(find(bbf.corrections(:,k)))>1)
            error(['Overlapping of BBFB correction signals ' ...
                   'in feedback vectors\nCheck BBFB ' ...
                   'correction configuration']);
        end;
    end
    if (size(bbf.corrections,2)~=length(ki))
        error('Number of gain values inconsistent with number of actuators');end

bbf.ki = zeros(size(bbf.corrections,2));
for c=1:length(ki)
    bbf.ki(c,c) = ki(c);
end

[bbf.idx_meas,discard] = find(bbf.measurements);

bbf.dv = [-.05 .05];
bbf.dphi = [-.05 .05];

[M] = dc_matrix(params_doublecomp, charge_nom, bbf);

[Ipk,sz_0,ddE_0,sd,dt,sdsgn,k,Eloss] = double_compressxv_better(params_doublecomp,0,0,0,0,0,0,[0 0 0 0 0],[0 0 0 0 0],charge_nom);

bbf.M = M;
bbf.sz_0 = sz_0;

end
